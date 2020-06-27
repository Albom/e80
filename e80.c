#include "system/kolibri.h"
#include "system/stdlib.h"
#include "system/string.h"
#include "system/msgbox.h"
#include "z80/z80.h"
#define TYPE_NO 0
#define TYPE_SNA 1
#define TYPE_Z80 2
#define SCREEN_LEN 3 * 3 *256 * 192 *3

char WND_CAPTION[] = "e80";

extern char _KOL_PARAM[256];
extern char _KOL_PATH[256];
extern char ROM[];

char szBackup[256];
char szScreen[256];

int fila[5][5];
int main_tecla, hay_tecla;
int SSCS = 0;

int frame_counter;
int target_cycle;
Z80Regs spectrumZ80;

char *screen;
unsigned screen_w, screen_h;

#define screen_a_w 512
#define screen_a_h 384
int flash = 0;
unsigned time = 0;

int get_ext(char *filename)
{
    return TYPE_SNA;
}

void memory_print(Z80Regs *regs, char *filename)
{
    kol_struct70 file;

    file.p00 = 2;
    file.p04 = 0;
    file.p12 = 64 * 1024;
    file.p16 = (unsigned)(regs->RAM);
    file.p20 = 0;
    file.p21 = filename;

    kol_file_70(&file);
}

void all_print(Z80Regs *regs, char *filename)
{
    kol_struct70 file;

    file.p00 = 2;
    file.p04 = 0;
    file.p12 = sizeof(Z80Regs);
    file.p16 = (unsigned) regs;
    file.p20 = 0;
    file.p21 = filename;

    kol_file_70(&file);
}

void screen_print(Z80Regs *regs)
{
    kol_struct70 file;

    const char palette[] = { 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xB0, 0x00, 0x00, 0xB0, 0x00, 0x00,
        0x00, 0x00, 0xB0, 0x00, 0x00, 0xB0,
        0xB0, 0x00, 0xB0, 0xB0, 0x00, 0xB0,
        0x00, 0xB0, 0x00, 0x00, 0xB0, 0x00,
        0xB0, 0xB0, 0x00, 0xB0, 0xB0, 0x00,
        0x00, 0xB0, 0xB0, 0x00, 0xB0, 0xB0,
        0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0,

        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
        0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF,
        0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
        0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00,
        0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    };

    char scr[6144];
    char atr[768];

    char a, c, s;
    unsigned bri;
    char *color;
    char *addr;
    int n = 0;
    int z = 0;
    int y;

    memcpy(scr, regs->RAM + 0x4000, 6144);
    memcpy(atr, regs->RAM + 0x5800, 768);

    for (int j = 0; j < 3; j++)
        for (int i = 0; i < 8; i++)
            for (int k = 0; k < 8; k++)
                for (int l = 0; l < 32; l++)
                {
                    c = scr[j *2048 + k *256 + i *32 + l];
                    for (int m = 0; m < 8; m++)
                    {
                        s = (c & 128) >> 7;
                        a = atr[j *256 + i *32 + l];

                        if ((a & 64) == 64)
                            bri = 8;
                        else
                            bri = 0;

                        if (0 == s)
                        {
                            if (!(flash && (128 == (a & 128))))
                                color = &palette[6 *(bri + ((a >> 3) &7))];
                            else
                                color = &palette[6 *(bri + (a & 7))];

                            addr = screen + 2 *screen_a_w *3 *z + 2 * 3 * n;

                            for (y = 0; y < 2; y++)
                                memcpy(addr + y *screen_a_w *3,
                                    color, 6);
                        }
                        else
                        {
                            if (!(flash && (128 == (a & 128))))
                                color = &palette[6 *(bri + (a & 7))];
                            else
                                color = &palette[6 *(bri + ((a >> 3) &7))];

                            addr = screen + 2 *screen_a_w *3 *z + 2 * 3 * n;

                            for (y = 0; y < 2; y++)
                                memcpy(addr + y *screen_a_w *3,
                                    color, 6);
                        }

                        n++;
                        if (256 == n)
                        {
                            n = 0;
                            z++;
                        }

                        c <<= 1;
                    }
                }

    if (33 < (kol_time_tick() - time))
    {
        if (0 == flash)
            flash = 1;
        else
            flash = 0;
        time = kol_time_tick();
    }

}

void keyboard_process(unsigned key)
{
    switch (key)
    {
        case 42:    // L-Shift Down
        case 54:    // R-Shift Down
            fila[4][1] &= 0xFE;
            break;

        case 42 + 128:    // L-Shift Up
        case 54 + 128:    // R-Shift Up
            fila[4][1] |= 1;
            break;

        case 29:    // Ctrl Down
            fila[4][2] &= 0xFD;
            break;

        case 29 + 128:    // Ctrl Up
            fila[4][2] |= 2;
            break;

        case 56:    // Alt Down
            fila[4][1] &= 0xFE;
            fila[4][2] &= 0xFD;
            break;

        case 56 + 128:    // Alt Up
            fila[4][1] |= 1;
            fila[4][2] |= 2;
            break;

        case 28:    // Enter Down
            fila[3][2] &= 0xFE;
            break;

        case 28 + 128:    // Enter Up
            fila[3][2] |= 1;
            break;

        case 2:    // 1 Down
            fila[1][1] &= 0xFE;
            break;

        case 2 + 128:    // 1 Up
            fila[1][1] |= 1;
            break;

        case 3:    // 2 Down
            fila[1][1] &= 0xFD;
            break;

        case 3 + 128:    // 2 Up
            fila[1][1] |= 2;
            break;

        case 4:    // 3 Down
            fila[1][1] &= 0xFB;
            break;

        case 4 + 128:    // 3 Up
            fila[1][1] |= 4;
            break;

        case 5:    // 4 Down
            fila[1][1] &= 0xF7;
            break;

        case 5 + 128:    // 4 Up
            fila[1][1] |= 8;
            break;

        case 6:    // 5 Down
            fila[1][1] &= 0xEF;
            break;

        case 6 + 128:    // 5 Up
            fila[1][1] |= 16;
            break;

        case 7:    // 6 Down
            fila[1][2] &= 0xEF;
            break;

        case 7 + 128:    // 6 Up
            fila[1][2] |= 16;
            break;

        case 8:    // 7 Down
            fila[1][2] &= 0xF7;
            break;

        case 8 + 128:    // 7 Up
            fila[1][2] |= 8;
            break;

        case 9:    // 8 Down
            fila[1][2] &= 0xFB;
            break;

        case 9 + 128:    // 8 Up
            fila[1][2] |= 4;
            break;

        case 10:    // 9 Down
            fila[1][2] &= 0xFD;
            break;

        case 10 + 128:    // 9 Up
            fila[1][2] |= 2;
            break;

        case 11:    // 0 Down
            fila[1][2] &= 0xFE;
            break;

        case 11 + 128:    // 0 Up
            fila[1][2] |= 1;
            break;

        case 16:    // Q Down
        case 72: // UpArrow Down
            fila[2][1] &= 0xFE;
            break;

        case 16 + 128:    // Q Up
        case 72 + 128: // UpArrow Up
            fila[2][1] |= 1;
            break;

        case 17:    // W Down
            fila[2][1] &= 0xFD;
            break;

        case 17 + 128:    // W Up
            fila[2][1] |= 2;
            break;

        case 18:    // E Down
            fila[2][1] &= 0xFB;
            break;

        case 18 + 128:    // E Up
            fila[2][1] |= 4;
            break;

        case 19:    // R Down
            fila[2][1] &= 0xF7;
            break;

        case 19 + 128:    // R Up
            fila[2][1] |= 8;
            break;

        case 20:    // T Down
            fila[2][1] &= 0xEF;
            break;

        case 20 + 128:    // T Up
            fila[2][1] |= 16;
            break;

        case 21:    // Y Down
            fila[2][2] &= 0xEF;
            break;

        case 21 + 128:    // Y Up
            fila[2][2] |= 16;
            break;

        case 22:    // U Down
            fila[2][2] &= 0xF7;
            break;

        case 22 + 128:    // U Up
            fila[2][2] |= 8;
            break;

        case 23:    // I Down
            fila[2][2] &= 0xFB;
            break;

        case 23 + 128:    // I Up
            fila[2][2] |= 4;
            break;

        case 24:    // O Down
        case 75:  // Left Down
            fila[2][2] &= 0xFD;
            break;

        case 24 + 128:    // O Up
        case 75 + 128:  // Left Up
            fila[2][2] |= 2;
            break;

        case 25:    // P Down
        case 77:  // Right Down
            fila[2][2] &= 0xFE;
            break;

        case 25 + 128:    // P Up
        case 77 + 128:  // Right Up
            fila[2][2] |= 1;
            break;

        case 30:    // A Down
        case 80: // DownArrow Down
            fila[3][1] &= 0xFE;
            break;

        case 30 + 128:    // A Up
        case 80 + 128: // DownArrow Up
            fila[3][1] |= 1;
            break;

        case 31:    // S Down
            fila[3][1] &= 0xFD;
            break;

        case 31 + 128:    // S Up
            fila[3][1] |= 2;
            break;

        case 32:    // D Down
            fila[3][1] &= 0xFB;
            break;

        case 32 + 128:    // D Up
            fila[3][1] |= 4;
            break;

        case 33:    // F Down
            fila[3][1] &= 0xF7;
            break;

        case 33 + 128:    // F Up
            fila[3][1] |= 8;
            break;

        case 34:    // G Down
            fila[3][1] &= 0xEF;
            break;

        case 34 + 128:    // G Up
            fila[3][1] |= 16;
            break;

        case 35:    // H Down
            fila[3][2] &= 0xEF;
            break;

        case 35 + 128:    // H Up
            fila[3][2] |= 16;
            break;

        case 36:    // J Down
            fila[3][2] &= 0xF7;
            break;

        case 36 + 128:    // J Up
            fila[3][2] |= 8;
            break;

        case 37:    // K Down
            fila[3][2] &= 0xFB;
            break;

        case 37 + 128:    // K Up
            fila[3][2] |= 4;
            break;

        case 38:    // L Down
            fila[3][2] &= 0xFD;
            break;

        case 38 + 128:    // L Up
            fila[3][2] |= 2;
            break;

        case 44:    // Z Down
            fila[4][1] &= 0xFD;
            break;

        case 44 + 128:    // Z Up
            fila[4][1] |= 2;
            break;

        case 45:    // X Down
            fila[4][1] &= 0xFB;
            break;

        case 45 + 128:    // X Up
            fila[4][1] |= 4;
            break;

        case 46:    // C Down
            fila[4][1] &= 0xF7;
            break;

        case 46 + 128:    // C Up
            fila[4][1] |= 8;
            break;

        case 47:    // V Down
            fila[4][1] &= 0xEF;
            break;

        case 47 + 128:    // V Up
            fila[4][1] |= 16;
            break;

        case 48:    // B Down
            fila[4][2] &= 0xEF;
            break;

        case 48 + 128:    // B Up
            fila[4][2] |= 16;
            break;

        case 49:    // N Down
            fila[4][2] &= 0xF7;
            break;

        case 49 + 128:    // N Up
            fila[4][2] |= 8;
            break;

        case 50:    // M Down
            fila[4][2] &= 0xFB;
            break;

        case 50 + 128:    // M Up
            fila[4][2] |= 4;
            break;

        case 57:    // Space Down
            fila[4][2] &= 0xFE;
            break;

        case 57 + 128:    // Space Up
            fila[4][2] |= 1;
            break;

        case 14:    // Backspace Down
            fila[1][2] &= 0xFE;
            fila[4][1] &= 0xFE;
            break;

        case 14 + 128:    // Backspace Up
            fila[1][2] |= 1;
            fila[4][1] |= 1;
            break;

        case 12:    // - Down
            fila[3][2] &= 0xF7;
            fila[4][2] &= 0xFD;
            break;

        case 12 + 128:    // - Up
            fila[3][2] |= 8;
            fila[4][2] |= 2;
            break;

        case 53:    // / Down
            fila[4][1] &= 0xEF;
            fila[4][2] &= 0xFD;
            break;

        case 53 + 128:    // / Up
            fila[4][1] |= 16;
            fila[4][2] |= 2;
            break;

        case 52:    // . Down
            fila[4][2] &= 0xFB;
            fila[4][2] &= 0xFD;
            break;

        case 52 + 128:    // . Up
            fila[4][2] |= 4;
            fila[4][2] |= 2;
            break;

        case 51:    //, Down
            fila[4][2] &= 0xF7;
            fila[4][2] &= 0xFD;
            break;

        case 51 + 128:    //, Up
            fila[4][2] |= 8;
            fila[4][2] |= 2;
            break;

        case 13:    // = Down
            fila[3][2] &= 0xFD;
            fila[4][2] &= 0xFD;
            break;

        case 13 + 128:    // = Up
            fila[3][2] |= 2;
            fila[4][2] |= 2;
            break;

    };

}

void memory_load_z80(Z80Regs *regs, char *filename)
{
    char header[30];
    kol_struct70 file;

    file.p00 = 0;
    file.p04 = 0;
    file.p12 = 30;
    file.p16 = (unsigned) header;
    file.p20 = 0;
    file.p21 = filename;
}

void memory_load_sna(Z80Regs *regs, char *filename)
{
    char buffer[27];
    kol_struct70 file;

    file.p00 = 0;
    file.p04 = 0;
    file.p12 = 27;
    file.p16 = (unsigned) buffer;
    file.p20 = 0;
    file.p21 = filename;

    kol_file_70(&file);

    regs->I = buffer[0];
    regs->HLs.B.l = buffer[1];
    regs->HLs.B.h = buffer[2];
    regs->DEs.B.l = buffer[3];
    regs->DEs.B.h = buffer[4];
    regs->BCs.B.l = buffer[5];
    regs->BCs.B.h = buffer[6];
    regs->AFs.B.l = buffer[7];
    regs->AFs.B.h = buffer[8];
    regs->HL.B.l = buffer[9];
    regs->HL.B.h = buffer[10];
    regs->DE.B.l = buffer[11];
    regs->DE.B.h = buffer[12];
    regs->BC.B.l = buffer[13];
    regs->BC.B.h = buffer[14];
    regs->IY.B.l = buffer[15];
    regs->IY.B.h = buffer[16];
    regs->IX.B.l = buffer[17];
    regs->IX.B.h = buffer[18];
    regs->IFF1 = regs->IFF2 = (buffer[19] &0x04) >> 2;
    regs->R.W = buffer[20];
    regs->AF.B.l = buffer[21];
    regs->AF.B.h = buffer[22];
    regs->SP.B.l = buffer[23];
    regs->SP.B.h = buffer[24];
    regs->IM = buffer[25];
    regs->BorderColor = buffer[26];

    file.p00 = 0;
    file.p04 = 27;
    file.p12 = 0x4000 * 3;
    file.p16 = (unsigned) regs->RAM + 0x4000;
    file.p20 = 0;
    file.p21 = filename;

    kol_file_70(&file);

    regs->PC.B.l = Z80MemRead(regs->SP.W, regs);
    regs->SP.W++;
    regs->PC.B.h = Z80MemRead(regs->SP.W, regs);
    regs->SP.W++;

}

void memory_save_sna(Z80Regs *regs, char *filename)
{
    char buffer[27];
    unsigned char sptmpl, sptmph;
    kol_struct70 file;

    buffer[0] = regs->I;
    buffer[1] = regs->HLs.B.l;
    buffer[2] = regs->HLs.B.h;
    buffer[3] = regs->DEs.B.l;
    buffer[4] = regs->DEs.B.h;
    buffer[5] = regs->BCs.B.l;
    buffer[6] = regs->BCs.B.h;
    buffer[7] = regs->AFs.B.l;
    buffer[8] = regs->AFs.B.h;
    buffer[9] = regs->HL.B.l;
    buffer[10] = regs->HL.B.h;
    buffer[11] = regs->DE.B.l;
    buffer[12] = regs->DE.B.h;
    buffer[13] = regs->BC.B.l;
    buffer[14] = regs->BC.B.h;
    buffer[15] = regs->IY.B.l;
    buffer[16] = regs->IY.B.h;
    buffer[17] = regs->IX.B.l;
    buffer[18] = regs->IX.B.h;
    buffer[19] = regs->IFF1 << 2;
    buffer[20] = regs->R.W & 0xFF;
    buffer[21] = regs->AF.B.l;
    buffer[22] = regs->AF.B.h;

    sptmpl = Z80MemRead(regs->SP.W - 1, regs);
    sptmph = Z80MemRead(regs->SP.W - 2, regs);

    Z80MemWrite(--(regs->SP.W), regs->PC.B.h, regs);
    Z80MemWrite(--(regs->SP.W), regs->PC.B.l, regs);

    buffer[23] = regs->SP.B.l;
    buffer[24] = regs->SP.B.h;
    buffer[25] = regs->IM;
    buffer[26] = regs->BorderColor;

    file.p00 = 2;
    file.p04 = 0;
    file.p12 = 27;
    file.p16 = (unsigned) buffer;
    file.p20 = 0;
    file.p21 = filename;

    kol_file_70(&file);

    file.p00 = 3;
    file.p04 = 27;
    file.p12 = 0x4000 * 3;
    file.p16 = (unsigned) regs->RAM + 0x4000;
    file.p20 = 0;
    file.p21 = filename;

    kol_file_70(&file);

    regs->SP.W += 2;
    Z80MemWrite(regs->SP.W - 1, sptmpl, regs);
    Z80MemWrite(regs->SP.W - 2, sptmph, regs);

}

void memory_save_scr(Z80Regs *regs, char *filename)
{
    kol_struct70 file;

    file.p00 = 2;
    file.p04 = 0x4000;
    file.p12 = 6912;
    file.p16 = (unsigned) regs->RAM + 0x4000;
    file.p20 = 0;
    file.p21 = filename;

    kol_file_70(&file);

}

void wnd_draw()
{
    kol_paint_start();
    kol_wnd_define((screen_w - 540) / 2, (screen_h - 440) / 2, 540, 440, 0x34b0b0b0, 0x34b0b0b0, WND_CAPTION);
    screen_print(&spectrumZ80);
    kol_paint_image((540 - screen_a_w) / 2 - 5,
        (440 - screen_a_h - kol_skin_height()) / 2,
        screen_a_w, screen_a_h, screen);
    kol_paint_end();
}

void kol_main()
{
    unsigned event;
    unsigned key;

    for (event = strlen(_KOL_PATH); event > 0; --event)
        if ('/' == _KOL_PATH[event])
        {
            _KOL_PATH[event + 1] = 0;
            break;
        }

    strcpy(szBackup, _KOL_PATH);
    strcpy(szScreen, _KOL_PATH);
    strcat(szBackup, "backup.sna");
    strcat(szScreen, "screen.scr");

    kol_screen_get_size(&screen_w, &screen_h);

    screen = malloc(SCREEN_LEN);
    spectrumZ80.RAM = (char*) malloc(64 *1024);
    memcpy(spectrumZ80.RAM, ROM, 16 *1024);

    Z80Reset(&spectrumZ80, 69888);
    Z80FlagTables();

    fila[1][1] = fila[1][2] = fila[2][2] = fila[3][2] = fila[4][2] =
        fila[4][1] = fila[3][1] = fila[2][1] = 0xFF;

    if (_KOL_PARAM != NULL)
    {
        int type = get_ext(_KOL_PARAM);

        if (TYPE_SNA == type)
            memory_load_sna(&spectrumZ80, _KOL_PARAM);
    }

    hay_tecla = main_tecla = 0;

    kol_key_mode_set(1);

    for (;;)
    {
        event = kol_event_wait_time(1);

        switch (event)
        {
            case 1:
                wnd_draw();
                break;

            case 2:
                key = (kol_key_get() >> 8) &0xff;

                switch (key)
                {
                    case 60:    // F2
                        if (IDOK == MessageBox("Save snapshot?", 
                            WND_CAPTION, MB_OKCANCEL))
                            {
                            memory_save_sna(&spectrumZ80,
                                szBackup);
                            }
                        break;

                    case 61:    // F3
                        if (IDOK == MessageBox("Load snapshot?", 
                            WND_CAPTION, MB_OKCANCEL))
                            {
                            memory_load_sna(&spectrumZ80,
                                szBackup);
                            }
                        break;

                    case 62:    // F4
                        if (IDOK == MessageBox("Save screenshot?", 
                            WND_CAPTION, MB_OKCANCEL))
                            {
                            memory_save_scr(&spectrumZ80,
                                szScreen);
                            }
                        break;

                    case 88:    // F12 Reset
                        if (IDOK == MessageBox("Reset?", 
                            WND_CAPTION, MB_OKCANCEL))
                            {
                            Z80Reset(&spectrumZ80, 69888);
                            Z80FlagTables();
                            fila[1][1] = fila[1][2] =
                                fila[2][2] = fila[3][2] =
                                fila[4][2] = fila[4][1] =
                                fila[3][1] = fila[2][1] = 0xFF;
                            }
                        break;

                    default:
                        keyboard_process(key);
                };

                break;

            case 3:
                if (1 == (kol_btn_get() &0xff00) >> 8)
                {
                    free(screen);
                    free(spectrumZ80.RAM);
                    kol_exit();
                }
                break;

            default:
                Z80Run(&spectrumZ80, 224 *64);
                for (int scanl = 0; scanl < 192; scanl++){
                    Z80Run(&spectrumZ80, 224);
                    }

                Z80Run(&spectrumZ80, 224 *56);

                if (target_cycle < 2 || frame_counter == 0)
                    {
                    screen_print(&spectrumZ80);
                    kol_screen_wait_rr();
                    kol_paint_image((540 - screen_a_w) / 2 - 5,
                        (440 - screen_a_h - kol_skin_height()) / 2,
                        screen_a_w, screen_a_h, screen);
                    }

                while (target_cycle == 0)
                {
                    target_cycle--;
                    frame_counter++;
                }

                break;

        };

    }
}

///=============================
