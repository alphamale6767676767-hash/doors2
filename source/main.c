// Hallway 100 v4 - original survival-horror hallway game for 3DS (citro2d)
// All art is drawn in code; all monsters, bosses and floors are original.
#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LAST_ROOM 100
#define SAVEMAGIC 0x48343030
#define ENDLESS (diff == 3)
#define NACH 14
#define NMON 10
#define NENT 15

typedef enum { TITLE, BESTIARY, ACHS, CARD, PLAY, OPENING, EVENT, DEAD, WIN } State;
enum { EV_LURKER, EV_GLARE, EV_SNARE, EV_WHISPER, EV_SKITTER, EV_MIMIC, EV_FLOOD, EV_DEVOUR, EV_PENDULUM, EV_GALE, EV_BOSS };

typedef struct { int magic, best, bestEndless, deaths, wins, runs, seen, bosses, ach; } Save;
static Save sv;

static State state = TITLE, afterCard = PLAY;
static int ev = 0, room = 1, t = 0, fr = 0, dodged = 0, gold = 0, lives = 3, maxLives = 3, diff = 1;
static int bandages = 0, skeletons = 0, shields = 0, battery = 100;
static bool lantern = false, hourglass = false, luckyCoin = false;
static bool locked, hasKey, closetRight, dark, shop, chest, chestOpen, fountain, fountainUsed;
static int lockType = 0, drawer[3];
static bool opened[3], pat[3], cur[3];
static int pal = 0, mash = 0, taps = 0, sel = 0, warnLen = 150;
static u32 snareKey, galeKey = KEY_DLEFT;
static const char *snareName = "X";
static u32 bseq[8];
static int blen = 4, bpos = 0, btime = 100, btimeMax = 100;
static char msg[80];
static int msgT = 0, flash = 0, achT = 0;
static const char *achText = "";
static float Z;
static C2D_TextBuf tb;

static const char *FLOORS[5] = { "THE MANOR", "THE LIBRARY", "THE GREENHOUSE", "THE CELLAR", "THE VOID" };
static const char *FLOORLORE[5] = {
    "An old house that forgot its owners.", "Books whisper if you stand too still.",
    "Plants grow where no sun has ever been.", "Damp stone. Dripping. Something below.",
    "The hallway ends. The dark does not."
};
static const char *DIFFN[4] = { "EASY (5 lives)", "NORMAL (3 lives)", "HARD (1 life)", "ENDLESS (no end)" };
static const char *EVN[NMON] = {
    "The Lurker", "The Glare", "The Snare", "The Whisperer", "The Skitter",
    "The Mimic", "The Flood", "The Devourer", "The Pendulum", "The Gale"
};
static const char *EVT[NMON] = {
    "Lights flicker, then it rushes. HOLD B to hide.",
    "A giant eye stares. HOLD L to close your eyes.",
    "A trap button flashes. Press it in time.",
    "It listens. Press NOTHING until it passes.",
    "It scuttles at you. MASH A 12 times!",
    "The closet is a lie. Do NOT hold B.",
    "Water rises. TAP the touch screen 10 times.",
    "Hold B and L together to survive.",
    "Press A when the marker hits the green zone.",
    "Wind howls. HOLD the arrow direction shown."
};
static const char *BOSSN[5] = { "The Hollow Butler", "The Archivist", "The Gardener", "The Drowned King", "The Void Warden" };
static const char *ACHN[NACH] = {
    "First Steps (reach room 2)", "Survivor (dodge 10 in one run)", "Butler Down (beat boss 1)",
    "Shopper (buy an item)", "Skeleton Crew (use a skeleton key)", "Green Thumb (reach the Greenhouse)",
    "Rich (hold 100 gold)", "Ghost Hunter (see every monster)", "Escape Artist (beat the game)",
    "Hard Mode Hero (beat it on HARD)", "Never Ending (room 120 Endless)", "Refreshed (use a fountain)",
    "Puzzle Master (solve a lever lock)", "Treasure Hunter (open a chest)"
};
static const char *NOTES[30] = {
    "Wallpaper peels in the shape of a door.", "Scratched: 'Don't trust the quiet.'",
    "A child's drawing of this hall, crooked.", "The air smells like wet pennies.",
    "A clock ticks, but there are no clocks.", "A note: 'The closet is safe. Mostly.'",
    "Your footsteps echo a half-second late.", "A cold draft slips under the next door.",
    "Dust shows no footprints but yours.", "A music box plays one note, then stops.",
    "The lights hum a tune you almost know.", "Someone has rearranged the furniture.",
    "Sign: 'Please keep the doors shut.'", "You swear the painting blinked.",
    "Wet footprints lead into the wall.", "The hall feels longer than before.",
    "A voice in the wall counts down.", "Candle wax drips from the ceiling. Upward.",
    "A coin in the carpet. It is warm.", "The door numbers don't match the rooms.",
    "A mirror shows the room, but not you.", "A faucet drips in perfect rhythm.",
    "Sticky note: 'Count your lives.'", "The shadows lean toward the door.",
    "A fly circles the lamp. Only thing alive.", "The floor creaks: three, then two.",
    "A tiny door in the baseboard is bricked up.", "You taste static at the back of your throat.",
    "Whoever lived here left in a hurry.", "The next door opens a crack. Then it shuts."
};
static const int PAL[5][4][3] = {
    {{100,80,60},{90,70,60},{80,60,45},{105,85,70}},
    {{70,55,40},{60,50,45},{85,65,45},{55,45,40}},
    {{60,95,60},{70,100,75},{55,85,70},{80,100,60}},
    {{70,75,85},{60,65,75},{80,80,70},{55,60,65}},
    {{70,40,100},{50,30,85},{90,40,90},{40,30,70}}
};
static const u32 BK[8] = { KEY_A, KEY_B, KEY_X, KEY_Y, KEY_DUP, KEY_DDOWN, KEY_DLEFT, KEY_DRIGHT };
static const u32 SK[7] = { KEY_X, KEY_Y, KEY_R, KEY_DUP, KEY_DDOWN, KEY_DLEFT, KEY_DRIGHT };
static const char *SAVEFILE = "sdmc:/hallway100.sav";

static u32 shade(int r, int g, int b, int k) { return C2D_Color32(r*k/100, g*k/100, b*k/100, 255); }
static u32 rgba(int r, int g, int b, int a) { return C2D_Color32(r, g, b, a); }
static void rect(float x, float y, float w, float h, u32 c) { C2D_DrawRectSolid(x, y, Z += 0.0005f, w, h, c); }
static void tri(float x0, float y0, float x1, float y1, float x2, float y2, u32 c) {
    C2D_DrawTriangle(x0, y0, c, x1, y1, c, x2, y2, c, Z += 0.0005f);
}
static void quad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, u32 c) {
    tri(x0, y0, x1, y1, x2, y2, c); tri(x0, y0, x2, y2, x3, y3, c);
}
static void text(const char *s, float x, float y, float sc, u32 c) {
    C2D_Text tx; C2D_TextParse(&tx, tb, s); C2D_TextOptimize(&tx);
    C2D_DrawText(&tx, C2D_WithColor, x, y, Z += 0.0005f, sc, sc, c);
}
static void setMsg(const char *s) { strncpy(msg, s, sizeof msg - 1); msg[sizeof msg - 1] = 0; msgT = 150; }
static void ach(int i) { if ((sv.ach >> i) & 1) return; sv.ach |= 1 << i; achText = ACHN[i]; achT = 200; }
static int floorOf(int r) { int f = (r - 1) / 20; return f < 0 ? 0 : (f > 4 ? 4 : f); }
static int bossNo(void) { int n = room / 20 - 1; return n < 0 ? 0 : n; }
static int bonus(void) { return (lantern ? 30 : 0) + (hourglass ? 30 : 0) + (diff == 0 ? 25 : (diff == 2 ? -15 : 0)); }
static int pendPos(int tt) { int p = (tt * 5) % 400; return p > 200 ? 400 - p : p; }

static const char *keyName(u32 k) {
    switch (k) {
        case KEY_A: return "A"; case KEY_B: return "B"; case KEY_X: return "X"; case KEY_Y: return "Y";
        case KEY_R: return "R"; case KEY_L: return "L";
        case KEY_DUP: return "UP"; case KEY_DDOWN: return "DOWN"; case KEY_DLEFT: return "LEFT"; case KEY_DRIGHT: return "RIGHT";
        default: return "?";
    }
}

static void loadAll(void) {
    Save tmp; memset(&sv, 0, sizeof sv);
    FILE *f = fopen(SAVEFILE, "rb");
    if (f) { if (fread(&tmp, sizeof tmp, 1, f) == 1 && tmp.magic == SAVEMAGIC) sv = tmp; fclose(f); }
    sv.magic = SAVEMAGIC;
}
static void saveAll(void) {
    if (ENDLESS) { if (room > sv.bestEndless) sv.bestEndless = room; }
    else { int r = room > LAST_ROOM ? LAST_ROOM : room; if (r > sv.best) sv.best = r; }
    FILE *f = fopen(SAVEFILE, "wb");
    if (f) { fwrite(&sv, sizeof sv, 1, f); fclose(f); }
}
static void checkBest(void) {
    if ((ENDLESS && room > sv.bestEndless) || (!ENDLESS && room <= LAST_ROOM && room > sv.best)) saveAll();
}

static void genBoss(void) {
    int bn = bossNo(); if (bn > 4) bn = 4;
    blen = 4 + bn;
    for (int i = 0; i < blen; i++) bseq[i] = BK[rand() % 8];
    bpos = 0;
    btimeMax = 110 - bn * 10 + bonus() / 2; if (btimeMax < 40) btimeMax = 40;
    btime = btimeMax;
}
static void startBoss(void) { ev = EV_BOSS; t = 0; state = EVENT; genBoss(); }

static void startEvent(void) {
    int f = floorOf(room), pool[16], n = 0;
    pool[n++] = EV_LURKER;
    if (room >= 6)  pool[n++] = EV_SNARE;
    if (room >= 10) pool[n++] = EV_GLARE;
    if (f >= 1) { pool[n++] = EV_WHISPER; pool[n++] = EV_SKITTER; }
    if (f >= 1 && room >= 25) pool[n++] = EV_PENDULUM;
    if (f >= 2) { pool[n++] = EV_MIMIC; pool[n++] = EV_GALE; }
    if (f >= 3) pool[n++] = EV_FLOOD;
    if (f >= 4) pool[n++] = EV_DEVOUR;
    ev = pool[rand() % n];
    warnLen = 150 - room; if (warnLen < 70) warnLen = 70;
    warnLen += bonus(); if (warnLen < 40) warnLen = 40;
    if (ev == EV_SNARE) { int k = rand() % 7; snareKey = SK[k]; snareName = keyName(snareKey); }
    if (ev == EV_GALE) galeKey = (rand() & 1) ? KEY_DLEFT : KEY_DRIGHT;
    sv.seen |= 1 << ev; mash = 0; taps = 0; t = 0; state = EVENT;
    if ((sv.seen & ((1 << NMON) - 1)) == ((1 << NMON) - 1)) ach(7);
}

static void enterRoom(void) {
    bool boss = (room % 20 == 0);
    t = 0; hasKey = false; chestOpen = false; fountainUsed = false; mash = 0; taps = 0;
    shop = (room % 10 == 5);
    locked = !shop && ((room % 10 == 0 && !boss) || (room >= 5 && rand() % 100 < 10));
    chest = !shop && !locked && !boss && rand() % 100 < 15;
    fountain = !shop && !locked && !chest && !boss && room >= 8 && rand() % 100 < 6;
    dark = floorOf(room) >= 1 && !shop && !boss && rand() % 100 < 25;
    pal = rand() % 4; closetRight = rand() & 1;
    lockType = (floorOf(room) >= 1 && rand() % 2) ? 1 : 0;
    int k = rand() % 3;
    for (int i = 0; i < 3; i++) {
        opened[i] = false; drawer[i] = (i == k) ? 1 : ((rand() % 2) ? 2 : 0);
        pat[i] = rand() & 1; cur[i] = false;
    }
    if (!pat[0] && !pat[1] && !pat[2]) pat[rand() % 3] = true;
    state = PLAY;
    if (rand() % 100 < 25) setMsg(NOTES[rand() % 30]);
    if (boss) startBoss();
    else if (!shop && room >= 3 && rand() % 100 < 30) startEvent();
    if ((room - 1) % 20 == 0) { afterCard = state; state = CARD; t = 0; }
    if (room >= 2) ach(0);
    if (room >= 41) ach(5);
    if (ENDLESS && room >= 120) ach(10);
}

static void newGame(void) {
    maxLives = diff == 0 ? 5 : (diff == 2 ? 1 : 3); lives = maxLives;
    room = 1; dodged = 0; gold = 0; bandages = 0; skeletons = 0; shields = 0;
    lantern = false; hourglass = false; luckyCoin = false;
    battery = 100; msgT = 0; flash = 0; sv.runs++;
    enterRoom();
}

static bool takeHit(void) {   // returns true if the run ended
    flash = 30;
    if (shields > 0) { shields--; setMsg("Your shield absorbed it!"); return false; }
    lives--; setMsg("Ouch! You lost a life.");
    if (lives <= 0) { state = DEAD; sv.deaths++; saveAll(); return true; }
    return false;
}
static void resolve(bool ok) {
    if (ok) {
        dodged++; gold += luckyCoin ? 5 : 2; setMsg(luckyCoin ? "You survived it! +5 gold" : "You survived it! +2 gold");
        state = PLAY; if (dodged >= 10) ach(1);
    } else { if (!takeHit()) state = PLAY; }
}
static void bossWin(void) {
    sv.bosses |= 1 << (bossNo() % 5); gold += 10; dodged++; ach(2);
    if (!ENDLESS && room >= LAST_ROOM) {
        state = WIN; sv.wins++; ach(8); if (diff == 2) ach(9); saveAll();
    } else { setMsg("Boss defeated! +10 gold"); state = PLAY; checkBest(); }
}
static void bossStep(u32 down) {
    if (t < 60) return;
    u32 pr = down & (KEY_A | KEY_B | KEY_X | KEY_Y | KEY_DUP | KEY_DDOWN | KEY_DLEFT | KEY_DRIGHT);
    bool miss = false;
    if (pr) {
        if (pr & bseq[bpos]) { bpos++; btime = btimeMax; if (bpos >= blen) { bossWin(); return; } }
        else miss = true;
    } else if (--btime <= 0) miss = true;
    if (miss) { if (!takeHit()) genBoss(); }
}

static void buy(int i) {
    static const int cost[6] = { 10, 15, 20, 40, 30, 25 };
    if (gold < cost[i]) { setMsg("Not enough gold."); return; }
    if (i == 0) bandages++;
    else if (i == 1) skeletons++;
    else if (i == 2) shields++;
    else if (i == 3) { if (lantern) { setMsg("You already own the lantern."); return; } lantern = true; }
    else if (i == 4) { if (hourglass) { setMsg("You already own the hourglass."); return; } hourglass = true; }
    else { if (luckyCoin) { setMsg("You already own the coin."); return; } luckyCoin = true; }
    gold -= cost[i]; setMsg("Purchased!"); ach(3);
}

static void drawDecor(int f, int k) {
    switch (f) {
        case 0:
            rect(148, 82, 16, 22, shade(110,80,40,k)); rect(150, 84, 12, 18, shade(60,90,120,k));
            rect(236, 82, 16, 22, shade(110,80,40,k)); rect(238, 84, 12, 18, shade(120,70,70,k));
            break;
        case 1:
            for (int i = 0; i < 8; i++) {
                rect(144 + i * 3.5f, 80, 3, 78, shade(80 + (i*37)%120, 40 + (i*53)%100, 40 + (i*29)%90, k));
                rect(228 + i * 3.5f, 80, 3, 78, shade(70 + (i*41)%120, 40 + (i*47)%100, 45 + (i*31)%90, k));
            }
            break;
        case 2:
            tri(146,170, 158,138, 170,170, shade(40,120,50,k)); tri(232,170, 244,134, 256,170, shade(50,130,60,k));
            rect(154, 160, 8, 10, shade(120,70,40,k)); rect(240, 160, 8, 10, shade(120,70,40,k));
            break;
        case 3:
            rect(140, 80, 120, 5, shade(120,120,130,k)); rect(140, 92, 120, 3, shade(100,100,110,k));
            rect(228, 140, 22, 30, shade(110,70,35,k)); rect(228, 150, 22, 3, shade(60,40,20,k));
            break;
        default:
            for (int i = 0; i < 14; i++)
                rect(140 + ((room*37 + i*53) % 120), 70 + ((room*91 + i*29) % 100), 2, 2, shade(220,200,255,k));
            break;
    }
}

static void drawHall(int k, float doorOpen, bool hiding) {
    int f = floorOf(room); const int *p = PAL[f][pal];
    rect(0, 0, 400, 240, rgba(0,0,0,255));
    quad(0,0, 400,0, 260,70, 140,70, shade(p[0],p[1],p[2],k*70/100));
    quad(0,240, 400,240, 260,170, 140,170, shade(p[0],p[1],p[2],k*45/100));
    quad(0,0, 140,70, 140,170, 0,240, shade(p[0],p[1],p[2],k*90/100));
    quad(400,0, 260,70, 260,170, 400,240, shade(p[0],p[1],p[2],k*90/100));
    rect(140, 70, 120, 100, shade(p[0],p[1],p[2],k));
    drawDecor(f, k);
    rect(178, 93, 44, 77, rgba(0,0,0,255));
    float w = 40 * (1.0f - doorOpen);
    rect(180, 95, w, 75, shade(130,85,40,k));
    if (w > 8) rect(180 + w - 8, 135, 4, 4, shade(220,200,80,k));
    if (locked && !hasKey && doorOpen == 0) rect(194, 120, 12, 14, shade(200,40,40,k));
    if (locked && lockType == 1 && !hasKey)
        for (int i = 0; i < 3; i++) rect(172 + i * 22, 76, 12, 12, pat[i] ? shade(255,230,80,k) : shade(50,50,50,k));
    if (shop) { rect(160, 48, 80, 18, shade(120,80,30,k)); text("SHOP", 182, 47, 0.55f, rgba(255,220,100,255)); }
    if (fountain) { rect(60, 150, 40, 14, shade(120,120,130,k)); rect(64, 144, 32, 8, shade(60,120,220,k)); }
    float cx = closetRight ? 325 : 20;
    rect(cx, 60, 55, 130, shade(60,40,30,k));
    rect(cx + 10, 70, 35, 110, shade(30,20,15,k));
    if (hiding) {
        rect(0, 0, 400, 95, rgba(0,0,0,255));
        rect(0, 135, 400, 105, rgba(0,0,0,255));
        for (int x = 0; x < 400; x += 40) rect(x, 95, 12, 40, rgba(0,0,0,255));
    }
}

int main(void) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget *bot = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    tb = C2D_TextBufNew(8192);
    srand(osGetTime());
    loadAll();

    while (aptMainLoop()) {
        hidScanInput();
        u32 down = hidKeysDown(), held = hidKeysHeld();
        touchPosition tp; hidTouchRead(&tp);
        if (down & KEY_START) break;
        fr++;

        bool inGame = (state == PLAY || state == EVENT);
        bool hiding = inGame && (held & KEY_B) && !(state == EVENT && ev == EV_BOSS);
        bool eyesClosed = (state == EVENT) && (ev == EV_GLARE || ev == EV_DEVOUR) && (held & KEY_L);
        bool light = false;
        if (dark && inGame && (held & KEY_SELECT) && battery > 0) { light = true; if (fr % 2 == 0) battery--; }
        else if (battery < 100 && fr % 6 == 0) battery++;
        if (inGame && gold >= 100) ach(6);

        // ---- touch: drawers / levers / chest / fountain / shop ----
        if (inGame && (down & KEY_TOUCH) && !(state == EVENT && (ev == EV_FLOOD || ev == EV_BOSS))) {
            if (locked && !hasKey) {
                for (int i = 0; i < 3; i++) {
                    int x = 15 + i * 100;
                    if (tp.px >= x && tp.px < x + 90 && tp.py >= 120 && tp.py < 190) {
                        if (lockType == 0) {
                            if (!opened[i]) {
                                opened[i] = true;
                                if (drawer[i] == 1) { hasKey = true; setMsg("Found the key! Open the door."); }
                                else if (drawer[i] == 2) { gold += 1 + rand() % 5; setMsg("Found some gold."); }
                                else setMsg("Empty.");
                            }
                        } else {
                            cur[i] = !cur[i];
                            if (cur[0] == pat[0] && cur[1] == pat[1] && cur[2] == pat[2]) {
                                hasKey = true; setMsg("Click! The lock opens."); ach(12);
                            }
                        }
                    }
                }
            } else if (chest && !chestOpen) {
                if (tp.px >= 110 && tp.px < 210 && tp.py >= 120 && tp.py < 180) {
                    chestOpen = true; gold += 3 + rand() % 10; setMsg("The chest holds gold!"); ach(13);
                }
            } else if (fountain && !fountainUsed) {
                if (tp.px >= 110 && tp.px < 210 && tp.py >= 120 && tp.py < 180) {
                    fountainUsed = true; ach(11);
                    if (lives < maxLives) { lives++; setMsg("The water restores you. +1 life"); }
                    else { gold += 3; setMsg("You find coins at the bottom. +3 gold"); }
                }
            } else if (shop) {
                for (int i = 0; i < 6; i++) {
                    int x = 15 + (i % 2) * 160, y = 78 + (i / 2) * 40;
                    if (tp.px >= x && tp.px < x + 145 && tp.py >= y && tp.py < y + 36) buy(i);
                }
            }
        }

        // ---- game logic ----
        switch (state) {
            case TITLE:
                if (down & KEY_DLEFT)  diff = diff > 0 ? diff - 1 : 0;
                if (down & KEY_DRIGHT) diff = diff < 3 ? diff + 1 : 3;
                if (down & KEY_X) { state = BESTIARY; sel = 0; }
                if (down & KEY_Y) state = ACHS;
                if (down & KEY_A) newGame();
                break;
            case BESTIARY:
                if (down & KEY_DLEFT)  sel = sel > 0 ? sel - 1 : NENT - 1;
                if (down & KEY_DRIGHT) sel = sel < NENT - 1 ? sel + 1 : 0;
                if (down & KEY_B) state = TITLE;
                break;
            case ACHS:
                if (down & KEY_B) state = TITLE;
                break;
            case CARD:
                if (++t >= 150 || (down & KEY_A)) { state = afterCard; t = 0; }
                break;
            case PLAY:
                if ((down & KEY_Y) && bandages > 0 && lives < maxLives) { bandages--; lives++; setMsg("Bandage used. +1 life"); }
                if ((down & KEY_A) && !hiding) {
                    if (locked && !hasKey && skeletons > 0) { skeletons--; hasKey = true; setMsg("Skeleton key used!"); ach(4); }
                    if (locked && !hasKey) setMsg(lockType == 0 ? "Locked! Search the drawers." : "Locked! Match the lights.");
                    else { state = OPENING; t = 0; }
                }
                break;
            case OPENING:
                if (++t >= 30) {
                    room++;
                    if (!ENDLESS && room > LAST_ROOM) { state = WIN; saveAll(); }
                    else { enterRoom(); checkBest(); }
                }
                break;
            case EVENT: {
                int b = bonus();
                t++;
                switch (ev) {
                    case EV_LURKER: if (t == warnLen + 90) resolve(hiding); break;
                    case EV_GLARE:  if (t == 210 + b) resolve(eyesClosed); break;
                    case EV_SNARE: {
                        u32 pr = down & (KEY_X | KEY_Y | KEY_R | KEY_DUP | KEY_DDOWN | KEY_DLEFT | KEY_DRIGHT);
                        if (t >= 60 && pr) resolve((pr & snareKey) != 0);
                        else if (t >= 150 + b) resolve(false);
                        break;
                    }
                    case EV_WHISPER: {
                        u32 any = down & (KEY_A | KEY_B | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_DUP | KEY_DDOWN | KEY_DLEFT | KEY_DRIGHT);
                        if (t >= 40 && any) resolve(false);
                        else if (t >= 180 + b) resolve(true);
                        break;
                    }
                    case EV_SKITTER:
                        if (t >= 20 && (down & KEY_A)) mash++;
                        if (mash >= 12) resolve(true); else if (t >= 170 + b) resolve(false);
                        break;
                    case EV_MIMIC: if (t == 200) resolve(!(held & KEY_B)); break;
                    case EV_FLOOD:
                        if (down & KEY_TOUCH) taps++;
                        if (taps >= 10) resolve(true); else if (t >= 200 + b) resolve(false);
                        break;
                    case EV_DEVOUR: if (t == 240 + b) resolve(hiding && (held & KEY_L)); break;
                    case EV_PENDULUM:
                        if (t >= 30 && (down & KEY_A)) { int ps = pendPos(t); resolve(ps >= 70 && ps <= 130); }
                        else if (t >= 210 + b) resolve(false);
                        break;
                    case EV_GALE:
                        if (t == 150 + b / 2) {
                            u32 other = (galeKey == KEY_DLEFT) ? KEY_DRIGHT : KEY_DLEFT;
                            resolve((held & galeKey) && !(held & other));
                        }
                        break;
                    case EV_BOSS: bossStep(down); break;
                    default: break;
                }
                break;
            }
            case DEAD: case WIN: if (down & KEY_A) state = TITLE; break;
        }

        // ---- brightness ----
        int k = 100;
        if (state == TITLE || state == BESTIARY || state == ACHS) k = 40;
        else if (state == PLAY || state == OPENING) k = (dark && !light) ? 18 : 100;
        else if (state == EVENT) {
            switch (ev) {
                case EV_LURKER:   k = (t < warnLen) ? (((t/5)%2) ? 100 : 12) : 18; break;
                case EV_GLARE:    k = (t < 120) ? (((t/8)%2) ? 100 : 25) : 30; break;
                case EV_SNARE:    k = (t < 60) ? (((t/3)%2) ? 100 : 15) : 60; break;
                case EV_WHISPER:  k = (t < 40) ? (((t/6)%2) ? 100 : 30) : 35; break;
                case EV_SKITTER:  k = (t < 20) ? (((t/4)%2) ? 100 : 20) : 45; break;
                case EV_MIMIC:    k = (t < 150) ? (((t/10)%2) ? 100 : 40) : 30; break;
                case EV_FLOOD:    k = 70; break;
                case EV_DEVOUR:   k = (t % 12 < 6) ? 25 : 10; break;
                case EV_PENDULUM: k = (t < 30) ? (((t/5)%2) ? 100 : 30) : 60; break;
                case EV_GALE:     k = 55; break;
                default:          k = 35; break;
            }
        }
        float door = (state == OPENING) ? t / 30.0f : 0.0f;

        // ================= DRAW =================
        Z = 0; C2D_TextBufClear(tb);
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        C2D_TargetClear(top, rgba(0,0,0,255));
        C2D_SceneBegin(top);
        char buf[112];

        if (state == CARD) {
            rect(0, 0, 400, 240, rgba(0,0,0,255));
            text(FLOORS[floorOf(room)], 80, 70, 1.3f, rgba(255,255,255,255));
            text(FLOORLORE[floorOf(room)], 80, 125, 0.55f, rgba(170,170,200,255));
            snprintf(buf, sizeof buf, "Rooms %d - %d", floorOf(room) * 20 + 1, (floorOf(room) + 1) * 20);
            text(buf, 140, 160, 0.6f, rgba(140,140,140,255));
        } else if (state == BESTIARY || state == ACHS) {
            rect(0, 0, 400, 240, rgba(10,10,15,255));
        } else {
            drawHall(k, door, hiding);
        }

        if (state == EVENT) {
            switch (ev) {
                case EV_LURKER:
                    if (t >= warnLen) {
                        float s = (t - warnLen) / 90.0f, sz = 20 + 340 * s;
                        rect(200 - sz/2, 120 - sz/2, sz, sz, rgba(40,0,60,200));
                        rect(200 - sz*0.25f, 120 - sz*0.1f, sz*0.1f, sz*0.1f, rgba(255,255,255,255));
                        rect(200 + sz*0.15f, 120 - sz*0.1f, sz*0.1f, sz*0.1f, rgba(255,255,255,255));
                    }
                    break;
                case EV_GLARE: case EV_DEVOUR:
                    if (ev == EV_GLARE && t >= 40) {
                        float sz = (t < 120) ? 10 + (t - 40) * 0.6f : 58 + (t - 120) * 2.0f;
                        rect(200 - sz, 110 - sz/2, sz*2, sz, rgba(240,240,240,255));
                        rect(200 - sz/4, 110 - sz/4, sz/2, sz/2, rgba(200,0,0,255));
                    }
                    if (ev == EV_DEVOUR) {
                        float g = t * 0.7f; if (g > 190) g = 190;
                        rect(0, 0, 400, g * 0.6f, rgba(0,0,0,230)); rect(0, 240 - g * 0.6f, 400, g * 0.6f, rgba(0,0,0,230));
                        rect(0, 0, g, 240, rgba(0,0,0,230)); rect(400 - g, 0, g, 240, rgba(0,0,0,230));
                        text("HIDE + CLOSE EYES  (B + L)", 70, 105, 0.8f, rgba(255,80,80,255));
                    }
                    if (eyesClosed) {
                        rect(0, 0, 400, 240, rgba(0,0,0,255));
                        text("(eyes closed)", 130, 110, 0.7f, rgba(120,120,120,255));
                    }
                    break;
                case EV_SNARE:
                    if (t >= 60) {
                        snprintf(buf, sizeof buf, "PRESS %s !", snareName);
                        text(buf, 90, 90, 1.6f, rgba(255,230,60,255));
                    }
                    break;
                case EV_WHISPER:
                    if (t >= 40) {
                        float sz = 30 + (t - 40) * 0.5f;
                        rect(200 - sz/2, 100 - sz/2, sz, sz * 1.3f, rgba(190,200,230,160));
                        rect(200 - sz/4, 95 - sz/6, 5, 8, rgba(0,0,0,255)); rect(200 + sz/4 - 5, 95 - sz/6, 5, 8, rgba(0,0,0,255));
                        text("shhh... stay still...", 110, 200, 0.8f, rgba(190,200,255,255));
                    }
                    break;
                case EV_SKITTER:
                    if (t >= 20) {
                        float s = (t - 20) / 150.0f, sz = 30 + 220 * s;
                        rect(200 - sz/2, 150 - sz/3, sz, sz * 0.5f, rgba(20,10,10,230));
                        for (int i = 0; i < 6; i++) rect(200 - sz/2 - 10 + i * (sz/5), 150 + sz/6, 4, sz * 0.4f, rgba(20,10,10,230));
                        snprintf(buf, sizeof buf, "MASH A!  %d / 12", mash);
                        text(buf, 120, 30, 0.9f, rgba(255,120,60,255));
                    }
                    break;
                case EV_MIMIC:
                    if (t > 100) {
                        float cx = closetRight ? 325 : 20;
                        rect(cx + 14, 90, 8, 6, rgba(255,0,0,255)); rect(cx + 34, 90, 8, 6, rgba(255,0,0,255));
                    }
                    break;
                case EV_FLOOD: {
                    float h = t * 0.6f; if (h > 220) h = 220;
                    rect(0, 240 - h, 400, h, rgba(30,80,200,150));
                    snprintf(buf, sizeof buf, "TAP THE SCREEN!  %d / 10", taps);
                    text(buf, 90, 30, 0.8f, rgba(255,255,255,255));
                    break;
                }
                case EV_PENDULUM:
                    if (t >= 30) {
                        int ps = pendPos(t);
                        rect(100, 196, 200, 12, rgba(60,60,70,255));
                        rect(170, 196, 60, 12, rgba(60,200,80,255));
                        rect(100 + ps - 2, 190, 5, 24, rgba(255,255,255,255));
                        text("PRESS A in the green!", 105, 150, 0.8f, rgba(255,230,120,255));
                    }
                    break;
                case EV_GALE: {
                    bool left = (galeKey == KEY_DLEFT);
                    for (int i = 0; i < 8; i++) {
                        float x = (float)((t * 14 + i * 53) % 400); if (left) x = 400 - x;
                        rect(x, 30 + i * 24, 36, 3, rgba(255,255,255,150));
                    }
                    snprintf(buf, sizeof buf, "HOLD %s !", left ? "LEFT" : "RIGHT");
                    text(buf, 110, 100, 1.3f, rgba(255,255,255,255));
                    break;
                }
                case EV_BOSS: {
                    int bn = bossNo() % 5;
                    rect(0, 0, 400, 240, rgba(20,0,0,235));
                    rect(150, 30, 100, 130, rgba(70 + bn * 20, 10, 20 + bn * 30, 255));
                    rect(170, 60, 14, 10, rgba(255,255,255,255)); rect(216, 60, 14, 10, rgba(255,255,255,255));
                    rect(175, 110, 50, 8, rgba(0,0,0,255));
                    text(BOSSN[bn], 120, 4, 0.8f, rgba(255,100,100,255));
                    if (t < 60) text("GET READY!", 140, 170, 0.8f, rgba(255,255,255,255));
                    else {
                        rect(20, 172, 360.0f * btime / (btimeMax > 0 ? btimeMax : 1), 6, rgba(220,40,40,255));
                        for (int i = 0; i < blen; i++) {
                            u32 c = i < bpos ? rgba(80,255,80,255) : (i == bpos ? rgba(255,230,60,255) : rgba(255,255,255,255));
                            text(keyName(bseq[i]), 20 + i * 46, i == bpos ? 190 : 198, i == bpos ? 0.9f : 0.65f, c);
                        }
                    }
                    break;
                }
                default: break;
            }
        }
        if (flash > 0) { rect(0, 0, 400, 240, rgba(200,0,0,flash * 6)); flash--; }

        if (state == TITLE) {
            text("HALLWAY 100", 70, 40, 1.7f, rgba(255,255,255,255));
            text("Press A to begin", 120, 120, 0.8f, rgba(220,220,220,255));
            snprintf(buf, sizeof buf, "< %s >", DIFFN[diff]);
            text(buf, 110, 160, 0.7f, rgba(255,200,100,255));
            text("X: Bestiary    Y: Achievements", 90, 195, 0.55f, rgba(180,180,180,255));
        }
        if (state == BESTIARY) {
            if (sel < NMON) {
                bool seen = (sv.seen >> sel) & 1;
                text(seen ? EVN[sel] : "???", 20, 30, 1.1f, rgba(255,255,255,255));
                text(seen ? EVT[sel] : "Not yet encountered.", 20, 90, 0.55f, rgba(200,200,200,255));
            } else {
                int b = sel - NMON;
                bool beat = (sv.bosses >> b) & 1;
                text(beat ? BOSSN[b] : "??? (Boss)", 20, 30, 1.1f, rgba(255,120,120,255));
                snprintf(buf, sizeof buf, "Boss of room %d. Repeat a %d-button sequence in time.", (b + 1) * 20, 4 + b);
                text(beat ? buf : "Defeat this boss to learn about it.", 20, 90, 0.5f, rgba(200,200,200,255));
            }
            snprintf(buf, sizeof buf, "Entry %d / %d", sel + 1, NENT);
            text(buf, 20, 200, 0.6f, rgba(150,150,150,255));
        }
        if (state == ACHS) {
            int got = 0; for (int i = 0; i < NACH; i++) if ((sv.ach >> i) & 1) got++;
            snprintf(buf, sizeof buf, "ACHIEVEMENTS  %d / %d", got, NACH);
            text(buf, 15, 4, 0.7f, rgba(255,255,255,255));
            for (int i = 0; i < NACH; i++) {
                bool g = (sv.ach >> i) & 1;
                snprintf(buf, sizeof buf, "[%c] %s", g ? 'x' : ' ', g ? ACHN[i] : "???");
                text(buf, 15, 28 + i * 14, 0.48f, g ? rgba(120,255,120,255) : rgba(130,130,130,255));
            }
        }
        if (state == DEAD) {
            rect(0, 0, 400, 240, rgba(120,0,0,230));
            text("YOU DIED", 110, 70, 1.5f, rgba(255,255,255,255));
            snprintf(buf, sizeof buf, "Reached room %d   Gold %d", room, gold);
            text(buf, 100, 140, 0.7f, rgba(255,220,220,255));
        }
        if (state == WIN) {
            rect(0, 0, 400, 240, rgba(0,0,0,255));
            text("YOU ESCAPED!", 90, 50, 1.5f, rgba(255,255,120,255));
            text("The Void Warden falls. The hallway ends.", 50, 120, 0.6f, rgba(220,220,220,255));
            snprintf(buf, sizeof buf, "Gold: %d   Monsters dodged: %d", gold, dodged);
            text(buf, 80, 150, 0.6f, rgba(220,220,220,255));
            text("Try ENDLESS mode on the title screen!", 60, 185, 0.55f, rgba(180,180,255,255));
        }
        if (achT > 0) {
            rect(30, 4, 340, 22, rgba(0,0,0,210));
            snprintf(buf, sizeof buf, "Achievement: %s", achText);
            text(buf, 36, 6, 0.5f, rgba(255,220,100,255));
            achT--;
        }

        // ---------- bottom screen ----------
        C2D_TargetClear(bot, rgba(15,15,20,255));
        C2D_SceneBegin(bot);
        if (state == TITLE) {
            snprintf(buf, sizeof buf, "Best: room %d/%d   Endless best: %d", sv.best, LAST_ROOM, sv.bestEndless);
            text(buf, 15, 15, 0.6f, rgba(255,255,255,255));
            snprintf(buf, sizeof buf, "Runs: %d   Deaths: %d   Escapes: %d", sv.runs, sv.deaths, sv.wins);
            text(buf, 15, 40, 0.5f, rgba(190,190,190,255));
            text("A: door   Hold B: hide   Hold L: close eyes", 15, 80, 0.5f, rgba(160,160,160,255));
            text("Y: bandage   SELECT: flashlight (dark rooms)", 15, 98, 0.5f, rgba(160,160,160,255));
            text("Tap drawers, levers, chests, fountains, shop", 15, 116, 0.5f, rgba(160,160,160,255));
            text("D-pad left/right: difficulty", 15, 134, 0.5f, rgba(160,160,160,255));
        } else if (state == BESTIARY) {
            text("BESTIARY", 15, 15, 0.9f, rgba(255,255,255,255));
            text("D-pad left/right: browse", 15, 60, 0.55f, rgba(180,180,180,255));
            text("B: back", 15, 80, 0.55f, rgba(180,180,180,255));
        } else if (state == ACHS) {
            text("B: back", 15, 15, 0.6f, rgba(180,180,180,255));
        } else if (state == CARD) {
            text("Press A to continue", 80, 110, 0.7f, rgba(180,180,180,255));
        } else {
            if (ENDLESS) snprintf(buf, sizeof buf, "Room %d  (ENDLESS)", room);
            else snprintf(buf, sizeof buf, "Room %d / %d", room > LAST_ROOM ? LAST_ROOM : room, LAST_ROOM);
            text(buf, 15, 8, 0.8f, rgba(255,255,255,255));
            text(FLOORS[floorOf(room)], 200, 14, 0.5f, rgba(160,160,200,255));
            snprintf(buf, sizeof buf, "Lives:%d/%d  Shield:%d  Gold:%d", lives, maxLives, shields, gold);
            text(buf, 15, 34, 0.5f, rgba(210,210,210,255));
            snprintf(buf, sizeof buf, "Bandage(Y):%d  Keys:%d  Lantern:%s  Hourglass:%s", bandages, skeletons, lantern ? "Y" : "N", hourglass ? "Y" : "N");
            text(buf, 15, 50, 0.42f, rgba(170,170,170,255));
            if (dark && inGame) {
                snprintf(buf, sizeof buf, "Dark room - hold SELECT for light  [%d%%]", battery);
                text(buf, 15, 64, 0.42f, rgba(255,230,120,255));
            }
            if (locked && !hasKey && inGame) {
                if (lockType == 0) text("Door is locked - tap a drawer:", 15, 98, 0.5f, rgba(255,200,90,255));
                else text("Match the wall lights with the levers:", 15, 98, 0.5f, rgba(255,200,90,255));
                for (int i = 0; i < 3; i++) {
                    int x = 15 + i * 100;
                    if (lockType == 0) {
                        rect(x, 120, 90, 70, opened[i] ? rgba(40,28,20,255) : rgba(110,72,40,255));
                        if (!opened[i]) rect(x + 38, 150, 14, 6, rgba(220,200,80,255));
                        else text(drawer[i] == 1 ? "KEY" : (drawer[i] == 2 ? "GOLD" : "empty"), x + 18, 142, 0.55f, rgba(255,255,255,255));
                    } else {
                        rect(x, 120, 90, 70, rgba(45,45,60,255));
                        rect(x + 40, cur[i] ? 126 : 152, 10, 34, rgba(210,210,220,255));
                        text(cur[i] ? "ON" : "OFF", x + 30, 170, 0.5f, cur[i] ? rgba(255,230,80,255) : rgba(140,140,140,255));
                    }
                }
            } else if (locked && hasKey && inGame) {
                text("The lock is open.", 15, 98, 0.5f, rgba(120,255,120,255));
            } else if (chest && inGame) {
                text(chestOpen ? "The chest is empty." : "A chest! Tap it:", 15, 98, 0.5f, rgba(255,220,120,255));
                rect(110, 120, 100, 60, chestOpen ? rgba(60,40,20,255) : rgba(150,100,40,255));
                rect(110, 120, 100, 14, rgba(90,60,25,255)); rect(155, 135, 10, 10, rgba(230,210,90,255));
            } else if (fountain && inGame) {
                text(fountainUsed ? "The fountain has run dry." : "A fountain! Tap it to drink:", 15, 98, 0.5f, rgba(140,200,255,255));
                rect(110, 150, 100, 30, rgba(120,120,130,255));
                rect(118, 130, 84, 24, fountainUsed ? rgba(40,50,70,255) : rgba(60,130,230,255));
            } else if (shop && inGame) {
                static const char *IN[6] = { "Bandage 10g", "Skel. Key 15g", "Shield 20g", "Lantern 40g", "Hourglass 30g", "Lucky Coin 25g" };
                static const char *ID[6] = { "+1 life (Y)", "opens a lock", "blocks 1 hit", "+30 reaction", "+30 reaction", "5 gold/dodge" };
                text("SHOP - tap to buy:", 15, 64, 0.45f, rgba(255,220,120,255));
                for (int i = 0; i < 6; i++) {
                    int x = 15 + (i % 2) * 160, y = 78 + (i / 2) * 40;
                    rect(x, y, 145, 36, rgba(60,45,30,255));
                    text(IN[i], x + 8, y + 3, 0.5f, rgba(255,255,255,255));
                    text(ID[i], x + 8, y + 19, 0.4f, rgba(200,200,170,255));
                }
            }
            if (state == EVENT && ev != EV_BOSS) text(EVT[ev], 15, 192, 0.45f, rgba(255,90,90,255));
            if (state == EVENT && ev == EV_BOSS) text("BOSS! Press the buttons shown, in order!", 15, 192, 0.5f, rgba(255,90,90,255));
            if (msgT > 0) { text(msg, 15, 215, 0.5f, rgba(255,255,255,255)); msgT--; }
            if (state == DEAD || state == WIN) text("Press A to return to the title", 15, 110, 0.6f, rgba(255,255,255,255));
        }

        C3D_FrameEnd(0);
    }

    saveAll();
    C2D_TextBufDelete(tb);
    C2D_Fini(); C3D_Fini(); gfxExit();
    return 0;
}
