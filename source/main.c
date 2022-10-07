#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <3ds.h>
#include <citro2d.h>

C2D_Font font;

static void drawText(float x, float y, float size, char *string)
{
    C2D_Text g_staticText;
    C2D_TextBuf g_staticBuf = C2D_TextBufNew(4096); // support up to 4096 glyphs in the buffer

    // Parse the text string
    C2D_TextFontParse(&g_staticText, font, g_staticBuf, string);

    // Optimize the text string
    C2D_TextOptimize(&g_staticText);

    // draw text
    C2D_DrawText(&g_staticText, 0, x * 2, y * 2, 0.5f, size, size);

    // Delete the text buffer
    C2D_TextBufDelete(g_staticBuf);
}

static void drawTextf(float x, float y, float size, const char *format, ...)
{
    char string[256];
    va_list args;
    va_start(args, format);
    vsprintf(string, format, args);
    va_end(args);
    drawText(x, y, size, string);
}

static void sceneInit(void)
{
    font = C2D_FontLoad("romfs:/OpenSans.bcfnt");
}

static void sceneExit(void)
{
    // free the font
    C2D_FontFree(font);
}

int main()
{
    romfsInit();
    cfguInit(); // Allow C2D_FontLoadSystem to work
    // Initialize the libs
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    // Create screen
    C3D_RenderTarget *bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    // Initialize the scene
    sceneInit();

    nfcStartScanning(0x0);
    NFC_TagInfo nfcTag;
    NFC_TagState nfcState;
    bool scanned = false;
    u8 UID[4] = {0, 0, 0, 0};

    // Main loop
    while (aptMainLoop())
    {
        hidScanInput();

        // Respond to user input
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
            break; // break in order to return to hbmenu

        nfcGetTagState(&nfcState);
        if ((nfcState == NFC_TagState_InRange) && (scanned = false))
        {
            nfcGetTagInfo(&nfcTag);
            UID[0] = nfcTag.id[0];
            UID[1] = nfcTag.id[1];
            UID[2] = nfcTag.id[2];
            UID[3] = nfcTag.id[3];
            scanned = true;
        }
        if ((nfcState == NFC_TagState_OutOfRange))
        {
            scanned = false;
            nfcStartScanning(0x0);
        }

        // Render the scene
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(bottom, C2D_Color32(0x68, 0xB0, 0xD8, 0xFF));
        C2D_SceneBegin(bottom);
        drawText(10.0f, 10.0f, 1.0f, "NFC Reader");
        drawTextf(10.0f, 20.0f, 1.0f, "Last Tag UID: %02X:%02X:%02X:%02X", UID[0], UID[1], UID[2], UID[3]);
        drawTextf(10.0f, 30.0f, 1.0f, "CanScan: %i", (int)!scanned);
        C3D_FrameEnd(0);
    }

    // Deinitialize the scene
    sceneExit();

    // Deinitialize the libs
    C2D_Fini();
    C3D_Fini();
    romfsExit();
    cfguExit();
    gfxExit();
    return 0;
}