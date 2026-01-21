#ifndef NEO_ENGINE_H
#define NEO_ENGINE_H

/* Platform Detection */
#ifdef __TURBOC__
    #define PLATFORM_DOS 1
    #define INLINE_ASM 1
    #include <dos.h>
    #include <conio.h>
#else
    #define PLATFORM_DOS 0
    #define INLINE_ASM 0
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
#endif

#ifdef SDL2_H
    #define PLATFORM_SDL2 1
#else
    #define PLATFORM_SDL2 0
#endif

/* Basic Type Definitions */
typedef unsigned char byte;
typedef unsigned short word;
typedef short integer;
typedef long longint;
typedef int fix16;
typedef char boolean;
#define true 1
#define false 0

/* Vector and Math Types */
typedef struct {
    fix16 x, y;
} TVec2;

typedef struct {
    TVec2 min, max;
} TBounds;

typedef struct {
    fix16 x, y;
} LinePoint;

typedef struct {
    fix16 dx, dy, x, y, sx, sy, err, x0, y0, x1, y1;
    int didReturnFirst;
    int done;
} TLine;

/* Scan Code Enumeration */
typedef enum {
    kNone, kEsc, k1, k2, k3, k4, k5, k6, k7, k8, k9, k0, kMinus, kEqual,
    kBack, kTab, kQ, kW, kE, kR, kT, kY, kU, kI, kO, kP, kLBracket,
    kRBracket, kEnter, kCtrl, kA, kS, kD, kF, kG, kH, kJ, kK, kL, kColon,
    kQuote, kTilde, kLShift, kBackSlash, kZ, kX, kC, kV, kB, kN, kM, kComma,
    kPeriod, kSlash, kRShift, kPadStar, kAlt, kSpace, kCaps, kF1, kF2, kF3,
    kF4, kF5, kF6, kF7, kF8, kF9, kF10, kNum, kScroll, kHome, kUp, kPgUp,
    kPadMinus, kLf, kPad5, kRt, kPadPlus, kend, kDn, kPgDn, kIns, kDel,
    kSysReq, kUnknown55, kUnknown56, kF11, kF12
} scanCode;

/* Constants */
#define MAX_ENT 64
#define COLLISION_SOLID 1
#define ENTFLAG_ACTIVE 1
#define ENTFLAG_VISIBLE 2
#define MAX_EVENTS 64
#define MAX_LINES 64

/* Event System */
typedef enum {
    SE_NONE, SE_KEYDOWN, SE_KEYUP, SE_KEYCHAR
} eventType;

typedef struct {
    eventType eventType;
    int param;
    int param2;
} TEvent;

typedef struct {
    scanCode code;
    char ch;
} TKeyPress;

/* Sound System */
typedef struct {
    int* data;
    int size;
    int length;
} TSoundEffect;
typedef TSoundEffect* PSoundEffect;

/* Image System */
typedef struct {
    word Width;
    word Height;
#if PLATFORM_SDL2
    void* surface;
    void* texture;
#else
    byte* Data;
#endif
} image_t;
typedef image_t* pimage_t;

/* Palette System */
typedef struct {
#if PLATFORM_SDL2
    void* sdlPalette;
#else
    byte c[256][3];
#endif
} Palette;

/* Function Pointer Types */
typedef void (*UpdateProc)(int deltaTime);
typedef void (*DrawProc)(void);
typedef void (*Event_KeyDownProc)(scanCode sc);
typedef void (*Event_KeyUpProc)(scanCode sc);
typedef void (*Event_KeyCharProc)(char ch);

/* Rendering Function Pointers */
typedef void (*R_FillColorProc)(int c);
typedef void (*R_FillRectProc)(int x, int y, int w, int h, int color);
typedef void (*R_DrawSubImageTransparentProc)(image_t* img, int dstX, int dstY, int srcX, int srcY, int srcWidth, int srcHeight);
typedef void (*R_DrawSubImageOpaqueProc)(image_t* img, int dstX, int dstY, int srcX, int srcY, int srcWidth, int srcHeight);
typedef void (*R_DrawSpriteProc)(int x, int y, image_t* img);
typedef void (*R_AllocPaletteProc)(Palette* pal);
typedef void (*R_LoadPaletteProc)(char* filename, Palette* pal);
typedef void (*R_SetPaletteColorProc)(int index, int r, int g, int b);
typedef void (*R_SetPaletteProc)(Palette* pal);
typedef void (*R_DrawTextProc)(int x, int y, char* str);
typedef void (*R_DrawLineProc)(int x0, int y0, int x1, int y1, int r, int g, int b, int a);
typedef void (*R_SwapBuffersProc)(void);
typedef void (*R_InitProc)(void);
typedef void (*R_CloseProc)(void);

/* Buffer System */
typedef struct TBufferBase {
    void* userData;
    FILE* _file;
} TBufferBase;

typedef struct TBufferReader {
    void* userData;
    FILE* _file;
    long pos;
    void (*readData)(struct TBufferReader* reader, void* Data, int length);
    long (*getPos)(struct TBufferReader* reader);
    void (*closeProc)(struct TBufferBase* reader);
} TBufferReader;

typedef struct TBufferWriter {
    void (*writeData)(struct TBufferWriter* writer, void* Data, int length);
    void* userData;
    FILE* _file;
} TBufferWriter;

/* Datafile Entry */
typedef struct {
    byte Name[9];
    int offset;
} entry;

/* Function Declarations */
char* itoa(int i);
char* WordToString(word i);
char* StrToPChar(char* s);

void Console_Print(char* s);
void Console_DrawText(void);
void Console_Dump(void);
void Console_ToggleVisible(void);
int Console_IsVisible(void);
void Console_SetWriteStdOut(int b);

void Event_Add(eventType eventType, int param, int param2);
void Event_ProcessEvents(void);
void Event_SetKeyDownProc(Event_KeyDownProc proc);
void Event_SetKeyUpProc(Event_KeyUpProc proc);
int Event_GetKeyPress(TKeyPress* keyPress);
void Event_ClearKeypressQueue(void);

pimage_t Neo_Image_Load(char* filename);

int I_IsKeyDown(scanCode sc);
int I_WasKeyReleased(scanCode sc);
int I_WasKeyPressed(scanCode sc);

void Keybrd_Init(void);

void Loop_SetUpdateProc(UpdateProc proc);
void Loop_SetDrawProc(DrawProc proc);
void Loop_Run(void);
void Loop_Cancel(void);

int Neo_Mouse_IsAvailable(void);
void Neo_Mouse_Init(void);
void Neo_Mouse_GetStatus(word* x, word* y, word* buttons);

PSoundEffect SND_AllocSoundEffect(int length);
PSoundEffect SND_LoadSoundEffect(char* filename);
void SND_FreeSoundEffect(PSoundEffect soundEffect);
void SND_PlaySound(PSoundEffect snd);
void SND_Update(void);
int SND_IsPlaying(void);
void SND_Init(void);
void SND_Close(void);

void SYS_FlushStdIO(void);
void SYS_PollEvents(void);

void Neo_Timer_Init(void);
void Timer_Delay(longint ms);
longint Timer_GetTicks(void);

void Neo_Init(void);
void Neo_Shutdown(void);

/* Fixed-point Math Functions */
fix16 sal(fix16 x, byte n);
fix16 sar(fix16 x, byte n);
fix16 i2f(int x);
int f2i(fix16 x);
fix16 fix16Div(fix16 val1, fix16 val2);
fix16 fix16Mul(fix16 val1, fix16 val2);
fix16 fix16Sqrt(fix16 val);

void Line_Init(TLine* line, fix16 x0, fix16 y0, fix16 x1, fix16 y1);
void getPoint(TLine* line, LinePoint* pt);

void BoundsFromSize(TVec2 origin, TVec2 mins, TVec2 maxs, TBounds* out);
void BoundsCenter(TBounds bounds, TVec2* out);
int BoundsIntersectsBounds(TBounds b0, TBounds b1);
void UnionBounds(TBounds* Result, TBounds a, TBounds b);
void OffsetBounds(TBounds bounds, TVec2 offset, TBounds* out);
int BoundsContainsXY(TBounds a, fix16 x, fix16 y);


/* Buffer Functions */
byte Buf_ReadByte(struct TBufferReader* reader);
int Buf_ReadInt(struct TBufferReader* reader);
longint Buf_ReadLong(struct TBufferReader* reader);
void Buf_ReadData(struct TBufferReader* reader, void* Data, int length);
char* Buf_ReadString(struct TBufferReader* reader);
longint Buf_GetReadPos(struct TBufferReader* reader);
void Buf_CloseReader(struct TBufferReader* reader);
void Buf_WriteInt(struct TBufferWriter* writer, int Value);
void Buf_WriteLong(struct TBufferWriter* writer, longint Value);
void Buf_WriteData(struct TBufferWriter* writer, void* Data, int length);
void Buf_WriteString(struct TBufferWriter* writer, char* Value);
void Buf_CreateReaderForFile(struct TBufferReader* reader);
void Buf_CreateReaderForMemory(char* Data, struct TBufferReader* reader);

/* Datafile Functions */
int Datafile_Open(char* Name, FILE* f, int recSize);
void Datafile_Close(FILE* f);
void Datafile_Init(struct TBufferReader* reader);
void Datafile_InitWithFile(char* path);
void Datafile_InitWithMemory(char* Data);
int DataFile_OpenWithReader(char* Name, struct TBufferReader* reader);
void Datafile_ReadString(FILE* f, char* s);

/* External Variables */
extern R_FillRectProc R_FillRect;
extern R_FillColorProc R_FillColor;
extern R_DrawSubImageTransparentProc R_DrawSubImageTransparent;
extern R_DrawSubImageOpaqueProc R_DrawSubImageOpaque;
extern R_DrawSpriteProc R_DrawSprite;
extern R_AllocPaletteProc R_AllocPalette;
extern R_LoadPaletteProc R_LoadPalette;
extern R_SetPaletteColorProc R_SetPaletteColor;
extern R_SetPaletteProc R_SetPalette;
extern R_SwapBuffersProc R_SwapBuffers;
extern R_DrawTextProc R_DrawText;
extern R_DrawLineProc R_DrawLine;
extern R_InitProc R_Init;
extern R_CloseProc R_Close;

extern int shouldQuit;

/* Implementation Section */
#ifdef NEO_ENGINE_IMPLEMENTATION

/* Global Variables */
static unsigned long keys = 0;
static unsigned long prevKeys = 0;
static unsigned long pressedKeys = 0;
static scanCode lastKeyDown = kNone;

static Event_KeyDownProc _keyDownProc = NULL;
static Event_KeyUpProc _keyUpProc = NULL;
static Event_KeyCharProc _keyCharProc = NULL;
static char lastKeyChar = '\0';

static TEvent events[MAX_EVENTS];
static int event_head = 0, event_tail = 0;
static TKeyPress keyPresses[8];
static int keyPressCount = 0;
static int keyPressTail = 0;
static int _event_is_adding = 0;

static UpdateProc _updateProc = NULL;
static DrawProc _drawProc = NULL;
static int _done = 0;
static longint lastTime = 0;

static PSoundEffect curSound = NULL;
static int curSoundSample = 0;

static int _keyboard_did_init = 0;
static int _mouse_did_init = 0;
static int _timer_did_init = 0;
static int _neo_did_shutdown = 0;

#if PLATFORM_DOS
static void interrupt (*oldTimerInt)();
static void interrupt (*oldKeyInt)();
static int oldTimerTickCount = 0;
static int oldTimerTicks = 0;
static longint tickCount = 0;
static word accum = 0;
static word soundTicks = 0;
static word soundDebug = 0;
#endif

/* Console System */
static char msg[32][101];
static int p = 0;
static int visible = 0;
static int writeStdOut = 1;

/* Line Pool */

/* Datafile System */
static int _numEntries = 0;
static entry* _entries = NULL;
static longint _start = 0;
static int _isMemoryBlob = 0;
static void* _memoryBlob = NULL;
static char _path[256];

/* Rendering Function Pointers */
R_FillRectProc R_FillRect = NULL;
R_FillColorProc R_FillColor = NULL;
R_DrawSubImageTransparentProc R_DrawSubImageTransparent = NULL;
R_DrawSubImageOpaqueProc R_DrawSubImageOpaque = NULL;
R_DrawSpriteProc R_DrawSprite = NULL;
R_AllocPaletteProc R_AllocPalette = NULL;
R_LoadPaletteProc R_LoadPalette = NULL;
R_SetPaletteColorProc R_SetPaletteColor = NULL;
R_SetPaletteProc R_SetPalette = NULL;
R_SwapBuffersProc R_SwapBuffers = NULL;
R_DrawTextProc R_DrawText = NULL;
R_DrawLineProc R_DrawLine = NULL;
R_InitProc R_Init = NULL;
R_CloseProc R_Close = NULL;

int shouldQuit = 0;

/* Fixed-point Math Implementation */
fix16 sal(fix16 x, byte n) {
#if INLINE_ASM
    fix16 result;
    asm {
        mov ax, x
        mov cl, n
        sal ax, cl
        mov result, ax
    }
    return result;
#else
    return x * (1 << n);
#endif
}

fix16 sar(fix16 x, byte n) {
#if INLINE_ASM
    fix16 result;
    asm {
        mov ax, x
        mov cl, n
        sar ax, cl
        mov result, ax
    }
    return result;
#else
    return x / (1 << n);
#endif
}

fix16 i2f(int x) {
#if INLINE_ASM
    fix16 result;
    asm {
        mov ax, x
        mov cl, 4
        sal ax, cl
        mov result, ax
    }
    return result;
#else
    return x * 16;
#endif
}

int f2i(fix16 x) {
#if INLINE_ASM
    int result;
    asm {
        mov ax, x
        mov cl, 4
        sar ax, cl
        mov result, ax
    }
    return result;
#else
    return x / 16;
#endif
}

fix16 fix16Div(fix16 val1, fix16 val2) {
    return sal(val1, 2) / sar(val2, 2);
}

fix16 fix16Mul(fix16 val1, fix16 val2) {
    return sar(val1, 2) * sar(val2, 2);
}

fix16 fix16Sqrt(fix16 val) {
    /* TODO: Implement sqrt lookup */
    return 0;
}
/* Bounds Functions */
void BoundsFromSize(TVec2 origin, TVec2 mins, TVec2 maxs, TBounds* out) {
    out->min.x = origin.x + mins.x;
    out->min.y = origin.y + mins.y;
    out->max.x = origin.x + maxs.x;
    out->max.y = origin.y + maxs.y;
}

void BoundsCenter(TBounds bounds, TVec2* out) {
    out->x = (bounds.min.x + bounds.max.x) / 2;
    out->y = (bounds.min.y + bounds.max.y) / 2;
}



int BoundsContainsXY(TBounds a, fix16 x, fix16 y) {
    if ((x > a.max.x) || (x < a.min.x) || (y > a.max.y) || (y < a.min.y))
        return 0;
    return 1;
}



/* Utility Functions */
char* itoa(int i) {
    static char buffer[32];
    sprintf(buffer, "%d", i);
    return buffer;
}

char* WordToString(word i) {
    return itoa(i);
}

char* StrToPChar(char* s) {
    return s;
}

/* Console Implementation */
void Console_SetWriteStdOut(int b) {
    writeStdOut = b;
}

void Console_Print(char* s) {
    if (writeStdOut) {
        printf("%s\n", s);
#if PLATFORM_DOS
        printf("\033[0m");
        SYS_FlushStdIO();
#endif
    }
    strncpy(msg[p & 31], s, 100);
    msg[p & 31][100] = '\0';
    p++;
}

void Console_Dump(void) {
    int j;
    for (j = (p - 31); j < (p - 1); j++) {
        if (j >= 0) {
            printf("%s\n", msg[j & 31]);
#if PLATFORM_DOS
            printf("\033[0m");
#endif
        }
    }
}

void Console_DrawText(void) {
    /* TODO: Implement console drawing */
}

void Console_ToggleVisible(void) {
    visible = !visible;
}

int Console_IsVisible(void) {
    return visible;
}

/* Event System Implementation */
void Event_SetKeyDownProc(Event_KeyDownProc proc) {
    _keyDownProc = proc;
}

void Event_SetKeyUpProc(Event_KeyUpProc proc) {
    _keyUpProc = proc;
}

int _Event_Reserve(void) {
    event_head = (event_head + 1) & (MAX_EVENTS - 1);
    return event_head;
}

void _Event_Set(int idx, eventType eventType, int param, int param2) {
    events[idx].eventType = eventType;
    events[idx].param = param;
    events[idx].param2 = param2;
}

void Event_Add(eventType eventType, int param, int param2) {
    int nextHead;
    
    if (_event_is_adding) return;
    _event_is_adding = 1;

    nextHead = (event_head + 1) & (MAX_EVENTS - 1);

    if (nextHead != event_tail) {
        event_head = nextHead;
        _Event_Set(event_head, eventType, param, param2);
    }

    _event_is_adding = 0;
}

void Event_ProcessEvents(void) {
    TEvent e;

    while (event_tail != event_head) {
        event_tail = (event_tail + 1) & (MAX_EVENTS - 1);
        e = events[event_tail];

        switch (e.eventType) {
            case SE_NONE:
                break;

            case SE_KEYDOWN:
                printf("SE_KEYDOWN %d\n", e.param);
                lastKeyDown = (scanCode)e.param;
                keys |= (1 << lastKeyDown);

                if (_keyDownProc) _keyDownProc(lastKeyDown);
                break;

            case SE_KEYUP:
                printf("SE_KEYUP %d\n", e.param);
                keys &= ~(1 << ((scanCode)e.param));

                if (_keyUpProc) _keyUpProc((scanCode)e.param);
                break;

            case SE_KEYCHAR:
                if (_keyCharProc) _keyCharProc((char)e.param);

                lastKeyChar = (char)e.param;

                keyPresses[keyPressCount].code = (scanCode)e.param;
                keyPresses[keyPressCount].ch = (char)e.param2;
                keyPressCount++;
                keyPressCount = (keyPressCount & 7);
                break;

            default:
                break;
        }

        events[event_tail].eventType = SE_NONE;
    }
}

void Event_ClearKeypressQueue(void) {
    keyPressTail = 0;
    keyPressCount = 0;
    keyPresses[keyPressTail].code = kNone;
    keyPresses[keyPressTail].ch = '\0';
}

int Event_GetKeyPress(TKeyPress* keyPress) {
    keyPress->code = kNone;
    keyPress->ch = '\0';
    
    if (keyPressTail == keyPressCount) return 0;

    if ((keyPresses[keyPressTail].code != kNone) || (keyPresses[keyPressTail].ch != '\0')) {
        *keyPress = keyPresses[keyPressTail];
        keyPresses[keyPressTail].code = kNone;
        keyPresses[keyPressTail].ch = '\0';
        keyPressTail = (keyPressTail + 1) & 7;
        return 1;
    }
    
    return 0;
}

/* Input Functions */
int I_IsKeyDown(scanCode sc) {
    return (keys & (1 << sc)) || (pressedKeys & (1 << sc));
}

int I_WasKeyReleased(scanCode sc) {
    return (!(keys & (1 << sc))) && (prevKeys & (1 << sc));
}

int I_WasKeyPressed(scanCode sc) {
    return (keys & (1 << sc)) && (!(prevKeys & (1 << sc)));
}

/* Sound System Implementation */
void SND_Update(void) {
#if PLATFORM_DOS
    int freq;
    if (curSound != NULL) {
        curSoundSample++;

        if (curSoundSample == curSound->length - 1) {
            curSound = NULL;
            nosound();
        } else {
            freq = curSound->data[curSoundSample];
            if (freq == 0) {
                nosound();
            } else {
                sound(freq);
            }
        }
    }
#endif
}

PSoundEffect SND_AllocSoundEffect(int length) {
    PSoundEffect soundEffect;
    soundEffect = (PSoundEffect)malloc(sizeof(TSoundEffect));
    soundEffect->data = (int*)malloc(length * sizeof(int));
    
    soundEffect->size = length;
    soundEffect->length = length;
    
    return soundEffect;
}

void SND_FreeSoundEffect(PSoundEffect soundEffect) {
    free(soundEffect->data);
    free(soundEffect);
}

void SND_PlaySound(PSoundEffect snd) {
    if (snd == NULL) return;
    
    curSound = snd;
    curSoundSample = -1;
}

int SND_IsPlaying(void) {
    return curSound != NULL;
}

PSoundEffect SND_LoadSoundEffect(char* filename) {
    FILE* f;
    int length;
    PSoundEffect soundEffect;
    
    soundEffect = NULL;
    if (!Datafile_Open(filename, f, 1)) return NULL;
    
    fread(&length, sizeof(int), 1, f);
    
    soundEffect = SND_AllocSoundEffect(length);
    fread(soundEffect->data, sizeof(int), length, f);
    
    fclose(f);
    return soundEffect;
}

void SND_Init(void) {
}

void SND_Close(void) {
}

/* Timer System Implementation */
#if PLATFORM_DOS
void interrupt _DOS_Timer_Int(void) {
    asm { cli }
    
    oldTimerTickCount--;
    tickCount++;
    accum += 72;

    while (accum > 99) {
        tickCount++;
        accum -= 100;
    }

    if (oldTimerTickCount == 0) {
        asm {
            pushf
            call oldTimerInt
        }
        oldTimerTickCount = oldTimerTicks;
    } else {
        asm {
            mov al, 0x20
            out 0x20, al
        }
    }

    soundTicks--;
    if (soundTicks == 0) {
        soundTicks = 4;
        SND_Update();
    }
    
    asm { sti }
}

void Timer_SetClockRate(int bits) {
    long ticks;
    ticks = 65536 >> bits;
    oldTimerTicks = 1 << bits;
    oldTimerTickCount = oldTimerTicks;
    
    soundTicks = 1;
    accum = 1;
    
    asm {
        mov al, 0x36
        out 0x43, al
        mov al, byte ptr ticks
        out 0x40, al
        mov al, byte ptr ticks+1
        out 0x40, al
    }
}
#endif

void Neo_Timer_Init(void) {
    if (!_timer_did_init) {
#if PLATFORM_DOS
        oldTimerInt = getvect(0x08);
        setvect(0x08, _DOS_Timer_Int);
        
        soundTicks = 1;
        Timer_SetClockRate(5);
        _timer_did_init = 1;
#endif
    }
}

void _Timer_Shutdown(void) {
    if (_timer_did_init) {
        _timer_did_init = 0;
#if PLATFORM_DOS
        Timer_SetClockRate(0);
        setvect(0x08, oldTimerInt);
        nosound();
#endif
    }
}

longint Timer_GetTicks(void) {
#if PLATFORM_DOS
    return tickCount;
#else
    /* TODO: Implement for other platforms */
    return 0;
#endif
}

void Timer_Delay(longint ms) {
    if (!_timer_did_init) return;
    
    longint t = Timer_GetTicks();
    while (Timer_GetTicks() - t < ms) {
        /* Wait */
    }
}

/* Game Loop Implementation */
void Loop_SetUpdateProc(UpdateProc proc) {
    _updateProc = proc;
}

void Loop_SetDrawProc(DrawProc proc) {
    _drawProc = proc;
}

void Loop_Cancel(void) {
    _done = 1;
}

void Loop_Run(void) {
#if PLATFORM_DOS
    longint lastFrameTime, frameTime, fpsTimer;
    int fpsCount, numUpdates;
    double accum, accum_saved;
    const double dt = 1000.0 / 60.0;
    const int dt_int = 16;
    
    _done = 0;
    fpsCount = 0;
    accum = 0;
    lastTime = Timer_GetTicks();
    fpsTimer = lastTime;
    lastFrameTime = lastTime;
    
    do {
        SYS_PollEvents();
        Event_ProcessEvents();
        
        numUpdates = 0;
        frameTime = Timer_GetTicks();
        
        accum = accum + (frameTime - lastFrameTime);
        
        if (accum > 2000) accum = 2000;
        
        accum_saved = accum;
        
        while (accum >= dt) {
            numUpdates++;
            _updateProc(dt_int);
            accum = accum - dt;
        }
        
        _drawProc();
        
        lastFrameTime = frameTime;
        
        fpsCount++;
        if (frameTime - fpsTimer >= 1000) {
            fpsTimer = frameTime;
            fpsCount = 0;
        }
        
        prevKeys = keys;
    } while (!_done && !shouldQuit);
#else
    /* TODO: Implement for other platforms */
    while (!_done && !shouldQuit) {
        SYS_PollEvents();
        Event_ProcessEvents();
        
        if (_updateProc) _updateProc(16);
        if (_drawProc) _drawProc();
    }
#endif
}

/* System Functions */
void SYS_FlushStdIO(void) {
#if PLATFORM_DOS
    /* TODO: Implement if needed */
#endif
}

void SYS_PollEvents(void) {
    /* Platform-specific event polling */
}

/* Mouse System Implementation */
#if PLATFORM_DOS
void interrupt _DOS_Mouse_Int(void) {
    asm {
        push ds
        push ax
        mov ax, seg _data
        mov ds, ax
        pop ax
        pop ds
    }
}
#endif

int Neo_Mouse_IsAvailable(void) {
#if PLATFORM_DOS
    union REGS regs;
    regs.x.ax = 0x0000;
    int86(0x33, &regs, &regs);
    return regs.x.ax != 0;
#else
    return 1;
#endif
}

void Neo_Mouse_Init(void) {
    if (_mouse_did_init || !Neo_Mouse_IsAvailable()) return;
    
    _mouse_did_init = 1;
#if PLATFORM_DOS
    union REGS regs;
    regs.x.ax = 0x000C;
    regs.x.cx = 0x001F;
    regs.x.dx = (unsigned int)_DOS_Mouse_Int;
    regs.x.es = (unsigned int)_DOS_Mouse_Int >> 16;
    int86(0x33, &regs, &regs);
    
    regs.x.ax = 0x0001;
    int86(0x33, &regs, &regs);
#endif
}

void Neo_Mouse_GetStatus(word* x, word* y, word* buttons) {
#if PLATFORM_DOS
    /* TODO: Implement DOS mouse status */
    *x = 0;
    *y = 0;
    *buttons = 0;
#else
    /* TODO: Implement for other platforms */
    *x = 0;
    *y = 0;
    *buttons = 0;
#endif
}

void _Mouse_Shutdown(void) {
    if (_mouse_did_init) {
        _mouse_did_init = 0;
#if PLATFORM_DOS
        union REGS regs;
        regs.x.ax = 0x0002;
        int86(0x33, &regs, &regs);
        
        regs.x.ax = 0x000C;
        regs.x.cx = 0x0000;
        regs.x.dx = 0x0000;
        regs.x.es = 0x0000;
        int86(0x33, &regs, &regs);
#endif
    }
}

/* Keyboard System Implementation */
#if PLATFORM_DOS
void interrupt _DOS_keyISR(void) {
    scanCode k;
    byte b;
    byte keyChar = 0, scanCode1 = 0;
    int event_index0;
    
    asm { cli }
    
    keyChar = 0;
    scanCode1 = 0;
    
    asm {
        in al, 0x60
        mov b, al
        and al, 0x7F
        mov k, al
        pushf
        call oldKeyInt
        
        mov ax, 0x0100
        int 0x16
        jz no_key
        
        mov ax, 0x0000
        int 0x16
        mov scanCode1, ah
        mov keyChar, al
    no_key:
    }
    
    if ((b & 0x80) == 0) {
        event_index0 = _Event_Reserve();
    }
    
    asm { sti }
    
    if ((b & 0x80) == 0) {
        keys |= (1 << k);
        pressedKeys |= (1 << k);
        
        if (keyChar != 0) {
            _Event_Set(event_index0, SE_KEYCHAR, 0, keyChar);
        }
    } else {
        keys &= ~(1 << k);
    }
    
    /* Clear keyboard buffer */
    asm {
        mov ax, 0x40
        mov es, ax
        mov word ptr es:0x1A, 0x1C
    }
}
#endif

void Keybrd_Init(void) {
    if (!_keyboard_did_init) {
        _keyboard_did_init = 1;
        keys = 0;
#if PLATFORM_DOS
        oldKeyInt = getvect(0x09);
        setvect(0x09, _DOS_keyISR);
#endif
    }
}

void _Keybrd_Shutdown(void) {
    if (_keyboard_did_init) {
        _keyboard_did_init = 0;
#if PLATFORM_DOS
        setvect(0x09, oldKeyInt);
#endif
    }
}

/* Image Loading */
pimage_t Neo_Image_Load(char* filename) {
    /* TODO: Implement image loading */
    return NULL;
}

/* Datafile System Implementation */
int Datafile_Open(char* Name, FILE* f, int recSize) {
    int i;
    
    for (i = 1; i <= _numEntries; i++) {
        if (strcmp(_entries[i].Name, Name) == 0) {
            strcpy(_path, Name);
            f = fopen(Name, "rb");
            if (f) {
                fseek(f, _start + _entries[i].offset, SEEK_SET);
                return 1;
            }
        }
    }
    
    Console_Print("DataFile_Open: did not find ");
    Console_Print(Name);
    
    char tempName[256];
    sprintf(tempName, "%s.bin", Name);
    f = fopen(tempName, "rb");
    return f != NULL;
}

void Datafile_Close(FILE* f) {
    if (f) fclose(f);
}

void Datafile_Init(struct TBufferReader* reader) {
    int i, l;
    
    _numEntries = Buf_ReadInt(reader);
    
    l = sizeof(entry) * _numEntries;
    _entries = (entry*)malloc(l);
    
    Console_Print("Datafile_Init: Number of entries: ");
    Console_Print(itoa(_numEntries));
    
    for (i = 1; i <= _numEntries; i++) {
        Buf_ReadData(reader, _entries[i].Name, 9);
        _entries[i].offset = Buf_ReadInt(reader);
    }
    
    _start = Buf_GetReadPos(reader);
}

void Datafile_InitWithFile(char* path) {
    struct TBufferReader reader;
    
    Console_Print("Datafile_InitWithFile: Using ");
    Console_Print(path);
    
    _isMemoryBlob = 0;
    strcpy(_path, path);
    
    reader._file = fopen(path, "rb");
    if (reader._file) {
        Buf_CreateReaderForFile(&reader);
        Datafile_Init(&reader);
        fclose(reader._file);
    }
}

void Datafile_InitWithMemory(char* Data) {
    struct TBufferReader reader;
    _isMemoryBlob = 1;
    _memoryBlob = Data;
    
    Buf_CreateReaderForMemory(Data, &reader);
    Datafile_Init(&reader);
}

int DataFile_OpenWithReader(char* Name, struct TBufferReader* reader) {
    int i;
    char localPath[256];
    
    sprintf(localPath, "mods/game/data/%s.bin", Name);
    reader->_file = fopen(localPath, "rb");
    if (reader->_file) {
        Console_Print("DataFile_OpenWithReader: local: ");
        Console_Print(localPath);
        Buf_CreateReaderForFile(reader);
        return 1;
    }
    
    for (i = 1; i <= _numEntries; i++) {
        if (strcmp(_entries[i].Name, Name) == 0) {
            if (_isMemoryBlob) {
                Buf_CreateReaderForMemory((char*)_memoryBlob, reader);
                reader->pos = _start + _entries[i].offset;
            } else {
                reader->_file = fopen(_path, "rb");
                if (reader->_file) {
                    fseek(reader->_file, _start + _entries[i].offset, SEEK_SET);
                    Buf_CreateReaderForFile(reader);
                    return 1;
                }
            }
            return 1;
        }
    }
    
    Console_Print("DataFile_OpenWithReader: did not find ");
    Console_Print(Name);
    return 0;
}

void Datafile_ReadString(FILE* f, char* s) {
    byte str_len;
    fread(&str_len, sizeof(byte), 1, f);
    fread(s, str_len, 1, f);
    s[str_len] = '\0';
}

/* Buffer System Implementation */
void _FileReadData(struct TBufferReader* reader, void* Data, int length) {
    fread(Data, 1, length, reader->_file);
}

longint _FileGetPos(struct TBufferReader* reader) {
    return ftell(reader->_file);
}

void _FileClose(struct TBufferBase* stream) {
    if (stream->_file) fclose(stream->_file);
}

void _MemoryReadData(struct TBufferReader* reader, void* Data, int length) {
    byte* bp = (byte*)reader->userData;
    bp += reader->pos;
    memcpy(Data, bp, length);
    reader->pos += length;
}

longint _MemoryGetPos(struct TBufferReader* reader) {
    return reader->pos;
}

byte Buf_ReadByte(struct TBufferReader* reader) {
    byte Value;
    reader->readData(reader, &Value, sizeof(byte));
    return Value;
}

int Buf_ReadInt(struct TBufferReader* reader) {
    int Value;
    reader->readData(reader, &Value, sizeof(int));
    return Value;
}

longint Buf_ReadLong(struct TBufferReader* reader) {
    longint Value;
    reader->readData(reader, &Value, sizeof(longint));
    return Value;
}

void Buf_ReadData(struct TBufferReader* reader, void* Data, int length) {
    reader->readData(reader, Data, length);
}

char* Buf_ReadString(struct TBufferReader* reader) {
    static char Value[256];
    byte len;
    
    reader->readData(reader, &len, 1);
    reader->readData(reader, Value, len);
    Value[len] = '\0';
    
    return Value;
}

void Buf_WriteInt(struct TBufferWriter* writer, int Value) {
    writer->writeData(writer, &Value, sizeof(int));
}

void Buf_WriteLong(struct TBufferWriter* writer, longint Value) {
    writer->writeData(writer, &Value, sizeof(longint));
}

void Buf_WriteData(struct TBufferWriter* writer, void* Data, int length) {
    writer->writeData(writer, Data, length);
}

void Buf_WriteString(struct TBufferWriter* writer, char* Value) {
    writer->writeData(writer, Value, strlen(Value) + 1);
}

longint Buf_GetReadPos(struct TBufferReader* reader) {
    return reader->getPos(reader);
}

void Buf_CreateReaderForFile(struct TBufferReader* reader) {
    reader->readData = _FileReadData;
    reader->getPos = _FileGetPos;
    reader->closeProc = (void(*)(struct TBufferBase*))_FileClose;
}

void Buf_CreateReaderForMemory(char* Data, struct TBufferReader* reader) {
    reader->pos = 0;
    reader->userData = Data;
    reader->readData = _MemoryReadData;
    reader->getPos = _MemoryGetPos;
    reader->closeProc = NULL;
}

void Buf_CloseReader(struct TBufferReader* reader) {
    if (reader->closeProc) {
        reader->closeProc((struct TBufferBase*)reader);
    }
}

/* Main Engine Functions */
void Neo_Init(void) {
    _neo_did_shutdown = 0;
    _keyboard_did_init = 0;
    _mouse_did_init = 0;
    _timer_did_init = 0;
    _event_is_adding = 0;
    
    event_head = 0;
    event_tail = 0;
    
    /* Initialize line pool */
    Line_InitPool();
}

void Neo_Shutdown(void) {
    if (!_neo_did_shutdown) {
        _neo_did_shutdown = 1;
        
        _Keybrd_Shutdown();
        _Mouse_Shutdown();
        
        SND_Close();
        _Timer_Shutdown();
        
        Console_Dump();
    }
}

#endif /* NEO_ENGINE_IMPLEMENTATION */

#endif /* NEO_ENGINE_H */