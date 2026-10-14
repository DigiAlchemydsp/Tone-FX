/* Where the mods keep their per-pattern settings so that they survive a power cycle.
 *
 * The firmware writes only parameter slots 0..45 of each sound to the +Drive, so the spare slots 46..52
 * are RAM-only: a setting kept there is lost at power-off. Every sound record does carry six bytes that
 * are saved and loaded but that the firmware itself never uses:
 *
 *   sound + 0x00..0x03   a reserved long, copied to and from the +Drive as is
 *   sound + 0x14..0x15   parameter slot 0, saved like the other parameters; no page edits it
 *
 * Measured: zero in all 1024 sound records on the +Drive (kits and the sound pool alike); written,
 * project saved, RAM scrubbed, project loaded - they come back; unchanged by the LEVEL knob, sample
 * changes and every knob of the SRC, FLTR, AMP and LFO pages; and the audio is bit-identical with
 * them set. (Two places tried first fail: the low byte of each track's level word is cleared by the
 * LEVEL knob, and the last four bytes of a sound are part of its sample reference, rewritten when the
 * sample changes and reset at load when the track has none.)
 *
 * So 6 bytes a track, 48 a pattern: byte n is in track n / 6's sound, at the n % 6-th of the offsets
 * above. Who owns what (zero, which every existing kit holds, means "not set" for all of them):
 *
 *   6t + 0, 6t + 1   Digi Matrix   slot t's routing, high byte then low byte
 *   6t + 2           Digi Matrix   slot t's depth + 128 (0 = none)
 *   6t + 3           Digi Poly     bit 7: track t is kept out of the POLY voice pool
 *                    Digi EQ       bits 0..6 (tracks 1..4 only): band t's type | Q << 3
 *   6t + 4, 6t + 5   Digi EQ       (tracks 1..4 only) band t: bit 15 stored, bit 14 (band 1 only) global,
 *                                  bits 7..12 level, bits 0..6 frequency
 *
 * Loading a sound onto a track replaces that sound's six bytes with the loaded sound's (zero). A load
 * also zeroes all 53 parameter slots of the sound first, RAM-only 46..52 included, which tells the two
 * cases apart: each mod keeps a marker in a RAM-only slot of its own. All 8 markers gone - the kit
 * was loaded (a project, a kit reload): the kit's bytes are the truth. Only some gone - sounds were
 * loaded onto those tracks: the mod puts its bytes back from its copy, so a sound load never changes
 * a POLY pool, a matrix slot or an EQ band.
 *
 * Over a power cycle the unit keeps its working project, unsaved edits included, in a non-volatile
 * mirror (0x406490f8..0x409bbbfc): each kit's eight sound records again, each written there by the
 * firmware's own serializer (0x4007a5a0, "Sound::updateMirror") whenever the firmware changes that sound.
 * The six bytes above are part of what it writes, but a mod writing them in RAM does not make the
 * firmware write the record again, so they were lost at power-off. ks_persist() calls the same
 * serializer for a sound whose bytes a mod changed. A kit's mirror: KS_MIRROR0 + pattern x 0xa00,
 * sound t at + t x 0xa0 (the kits: KS_KIT0 + pattern x 0x91e); it is written only when the record there
 * carries the serializer's own header (0xbeefbace, 3), so nothing is written anywhere else.
 */
#ifndef KITSTORE_H
#define KITSTORE_H

#define XB_N       48
#define XB_OFF(n)  (0x20 + (n) / 6 * 0xa2 + ((n) % 6 < 4 ? (n) % 6 : 0x10 + (n) % 6))
#define XB(kit, n) (*(volatile unsigned char *)(void *)((char *)(kit) + XB_OFF(n)))

/* RAM-only marker slots, one per mod */
#define KS_SLOT_EQ     50
#define KS_SLOT_MATRIX 51
#define KS_SLOT_POLY   52
#define KS_MARK        0x4b53
#define KS_MARKW(kit, t, slot) \
    (*(volatile unsigned short *)(void *)((char *)(kit) + 0x20 + (t) * 0xa2 + 0x14 + 2 * (slot)))

/* the non-volatile mirror of the kits (see above); the same serializer address on OS 1.53 and 1.54 */
#ifdef OS154
#define KS_KIT0       0x4191f018u
#define KS_MIRROR0    0x4095941cu
#else
#define KS_KIT0       0x4191e018u
#define KS_MIRROR0    0x4095841cu
#endif
#define KS_KIT_STEP   0x91eu
#define KS_MIRROR_STEP 0xa00u
#define KS_PATTERNS   128u
#define KS_SERIALIZE  ((int (*)(void *, void *, int))0x4007a5a0)

/* Write sound t of this kit to the unit's non-volatile mirror, as the firmware does after its own edits */
static void ks_persist(unsigned char *kit, int t)
{
    unsigned int off = (unsigned int)kit - KS_KIT0, p = off / KS_KIT_STEP;
    volatile unsigned int *m;
    if (off % KS_KIT_STEP || p >= KS_PATTERNS || (unsigned)t > 7)
        return;
    m = (volatile unsigned int *)(KS_MIRROR0 + p * KS_MIRROR_STEP + (unsigned)t * 0xa0u);
    if (m[0] != 0xbeefbaceu || m[1] != 3)
        return;
    KS_SERIALIZE((void *)m, kit + 0x20 + t * 0xa2, 0);
}

struct kstore {
    unsigned char *kit;                  /* the kit the copy is of */
    unsigned char copy[XB_N];            /* this mod's bytes as of the last tick */
};

enum { KS_SAME, KS_LOADED, KS_OTHER };

/* Once per UI frame. mask(n) gives the bits of byte n this mod owns. Returns KS_OTHER for another
 * pattern's kit, KS_LOADED when the whole kit was just loaded in place, else KS_SAME. */
static int kstore_tick(struct kstore *s, unsigned char *kit, int slot, unsigned char (*mask)(int))
{
    int t, n, lost = 0, r = KS_SAME, dirty = 0;
    if (!kit) {
        s->kit = 0;
        return KS_SAME;
    }
    for (t = 0; t < 8; t++)
        if (KS_MARKW(kit, t, slot) != KS_MARK)
            lost |= 1 << t;
    if (kit != s->kit)
        r = KS_OTHER;
    else if (lost == 0xff)
        r = KS_LOADED;
    else {
        if (lost)                                    /* single sounds loaded: this mod's bytes go back */
            for (n = 0; n < XB_N; n++) {
                unsigned char m = mask(n);
                if (m && ((lost >> (n / 6)) & 1)) {
                    XB(kit, n) = (unsigned char)((XB(kit, n) & ~m) | (s->copy[n] & m));
                    dirty |= 1 << (n / 6);           /* (the load wrote the sound's mirror without them) */
                }
            }
        for (n = 0; n < XB_N; n++)                   /* this mod's bytes changed since the last tick */
            if ((XB(kit, n) ^ s->copy[n]) & mask(n))
                dirty |= 1 << (n / 6);
    }
    for (t = 0; t < 8; t++)
        if ((lost >> t) & 1)
            KS_MARKW(kit, t, slot) = KS_MARK;
    for (t = 0; t < 8; t++)                          /* into the unit's non-volatile mirror */
        if ((dirty >> t) & 1)
            ks_persist(kit, t);
    s->kit = kit;
    for (n = 0; n < XB_N; n++)
        s->copy[n] = XB(kit, n);
    return r;
}

#endif
