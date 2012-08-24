    std::string renderer = std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    bool isPVR = renderer.find("SGX ") != std::string::npos;

    // Clear test
    ADD_TEST(ClearTest());

    // Normal blits
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              800, 480, "data/water2_800x480_rgba8888.raw"));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_BYTE,              800, 480, "data/water2_800x480_rgb888.raw"));
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              864, 480, "data/water2_864x480_rgba8888.raw"));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_BYTE,              864, 480, "data/water2_864x480_rgb888.raw"));
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,             1024, 512, "data/digital_nature2_1024x512_rgba8888.raw", false, 800.0 / 1024, 480.0 / 512));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,       800, 480, "data/water2_800x480_rgb565.raw"));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,      1024, 512, "data/digital_nature2_1024x512_rgb565.raw", false, 800.0 / 1024, 480.0 / 512));
    if (isPVR)
    {
        ADD_TEST(BlitTest(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, 0, 1024, 512, "data/abstract3_1024x512_pvrtc4.raw", false, 800.0 / 1024, 480.0 / 512));
        ADD_TEST(BlitTest(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, 0, 1024, 512, "data/abstract3_1024x512_pvrtc2.raw", false, 800.0 / 1024, 480.0 / 512));
    }
    ADD_TEST(BlitTest(GL_ETC1_RGB8_OES,                   0, 1024, 512, "data/abstract3_1024x512_etc1.raw", false, 800.0 / 1024, 480.0 / 512));
    ADD_TEST(BlitTest(GL_LUMINANCE, GL_UNSIGNED_BYTE,         800, 480, "data/abstract3_04_800x480_r8.raw"));
    ADD_TEST(BlitTest(GL_LUMINANCE, GL_UNSIGNED_BYTE,        1024, 512, "data/abstract3_04_1024x512_r8.raw", false, 800.0 / 1024, 480.0 / 512));
#if defined(SUPPORT_X11)
    ADD_TEST(PixmapBlitTest(w, h, ctx.config));
    ADD_TEST(PixmapBlitTest(w, h, config32));
#endif // SUPPORT_X11
    ADD_TEST(FBOBlitTest(GL_RGBA, GL_UNSIGNED_BYTE,          w, h));
    ADD_TEST(FBOBlitTest(GL_RGBA, GL_UNSIGNED_BYTE,          1024, 512, false, w / 1024, h / 512));
    ADD_TEST(FBOBlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,   w, h));
    ADD_TEST(FBOBlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,   1024, 512, false, w / 1042, h / 512));

    // Rotated blits
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              480,  800, "data/water2_480x800_rgba8888.raw", true));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_BYTE,              480,  800, "data/water2_480x800_rgb888.raw", true));
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              480,  864, "data/water2_480x864_rgba8888.raw", true));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_BYTE,              480,  864, "data/water2_480x864_rgb888.raw", true));
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              512, 1024, "data/digital_nature2_512x1024_rgba8888.raw", true, 480.0 / 512, 800.0 / 1024));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,       480,  800, "data/water2_480x800_rgb565.raw", true));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,       512, 1024, "data/digital_nature2_512x1024_rgb565.raw", true, 480.0 / 512, 800.0 / 1024));
    if (isPVR)
    {
        ADD_TEST(BlitTest(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, 0,  512, 1024, "data/abstract3_512x1024_pvrtc4.raw", true, 480.0 / 512, 800.0 / 1024));
        ADD_TEST(BlitTest(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, 0,  512, 1024, "data/abstract3_512x1024_pvrtc2.raw", true, 480.0 / 512, 800.0 / 1024));
    }
    ADD_TEST(BlitTest(GL_ETC1_RGB8_OES,                   0,  512, 1024, "data/abstract3_512x1024_etc1.raw", true, 480.0 / 512, 800.0 / 1024));
#if defined(SUPPORT_X11)
    ADD_TEST(PixmapBlitTest(h, w, ctx.config, true));
    ADD_TEST(PixmapBlitTest(h, w, config32,   true));
#endif // SUPPORT_X11
    ADD_TEST(FBOBlitTest(GL_RGBA, GL_UNSIGNED_BYTE,        w, h, true, h / w, w / h));
    ADD_TEST(FBOBlitTest(GL_RGBA, GL_UNSIGNED_BYTE,        1024, 512, true, h / 512, w / 1024));
    ADD_TEST(FBOBlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5, w, h, true, h / w, w / h));
    ADD_TEST(FBOBlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5, 1024, 512, true, h / 512, w / 1024));

#if defined(SUPPORT_X11)
    // Test composition performance
    ADD_TEST(BlitMultiTest<PixmapBlitTest>(w, h, ctx.config, false, 2));
    ADD_TEST(BlitMultiTest<PixmapBlitTest>(h, w, ctx.config, true, 2));
    ADD_TEST(BlitMultiTest<BlitTest>(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,       800, 480, "data/water2_800x480_rgb565.raw", false, 2));
    ADD_TEST(BlitMultiTest<BlitTest>(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,       800, 480, "data/water2_800x480_rgb565.raw", true, 2));
    ADD_TEST(BlitMultiTest<PixmapBlitTest>(w, h, config32,   false, 2));
    ADD_TEST(BlitMultiTest<PixmapBlitTest>(h, w, config32,   true, 2));
    ADD_TEST(BlitMultiTest<BlitTest>(GL_RGBA, GL_UNSIGNED_BYTE,              800, 480, "data/water2_800x480_rgba8888.raw", false, 2));
    ADD_TEST(BlitMultiTest<BlitTest>(GL_RGBA, GL_UNSIGNED_BYTE,              800, 480, "data/water2_800x480_rgba8888.raw", true, 2));
    ADD_TEST(BlitMultiTest<PixmapBlitTest>(w, h, ctx.config, false, 4));
    ADD_TEST(BlitMultiTest<PixmapBlitTest>(h, w, ctx.config, true, 4));
    ADD_TEST(BlitMultiTest<BlitTest>(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,       800, 480, "data/water2_800x480_rgb565.raw", false, 4));
    ADD_TEST(BlitMultiTest<BlitTest>(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,       800, 480, "data/water2_800x480_rgb565.raw", true, 4));
    ADD_TEST(BlitMultiTest<PixmapBlitTest>(w, h, config32,   false, 4));
    ADD_TEST(BlitMultiTest<PixmapBlitTest>(h, w, config32,   true, 4));
    ADD_TEST(BlitMultiTest<BlitTest>(GL_RGBA, GL_UNSIGNED_BYTE,              800, 480, "data/water2_800x480_rgba8888.raw", false, 4));
    ADD_TEST(BlitMultiTest<BlitTest>(GL_RGBA, GL_UNSIGNED_BYTE,              800, 480, "data/water2_800x480_rgba8888.raw", true, 4));
    ADD_TEST(BlitMultiTest<PixmapBlitTest>(w, h, ctx.config, false, 8));
    ADD_TEST(BlitMultiTest<PixmapBlitTest>(h, w, ctx.config, true, 8));
    ADD_TEST(BlitMultiTest<BlitTest>(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,       800, 480, "data/water2_800x480_rgb565.raw", false, 8));
    ADD_TEST(BlitMultiTest<BlitTest>(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,       800, 480, "data/water2_800x480_rgb565.raw", true, 8));
    ADD_TEST(BlitMultiTest<PixmapBlitTest>(w, h, config32,   false, 8));
    ADD_TEST(BlitMultiTest<PixmapBlitTest>(h, w, config32,   true, 8));
    ADD_TEST(BlitMultiTest<BlitTest>(GL_RGBA, GL_UNSIGNED_BYTE,              800, 480, "data/water2_800x480_rgba8888.raw", false, 8));
    ADD_TEST(BlitMultiTest<BlitTest>(GL_RGBA, GL_UNSIGNED_BYTE,              800, 480, "data/water2_800x480_rgba8888.raw", true, 8));
#endif // SUPPORT_X11

    int gridW = 5;
    int gridH = 3;
    float w2 = winWidth  / gridW;
    float h2 = winHeight / gridH;

    // Small blended blits
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,     128, 128, "data/xorg_128x128_rgba4444.raw", false, gridW, gridH, 128.0 / w2, 128.0 / h2, true));
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              127, 127, "data/xorg_127x127_rgba8888.raw", false, gridW, gridH, 127.0 / w2, 127.0 / h2, true));
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              128, 128, "data/xorg_128x128_rgba8888.raw", false, gridW, gridH, 128.0 / w2, 128.0 / h2, true));
    ADD_TEST(BlitTest(GL_RGB, GL_UNSIGNED_SHORT_5_6_5,        127, 127, "data/xorg_127x127_rgb565.raw",   false, gridW, gridH, 127.0 / w2, 127.0 / h2, true));
    ADD_TEST(BlitTest(GL_RGB, GL_UNSIGNED_SHORT_5_6_5,        128, 128, "data/xorg_128x128_rgb565.raw",   false, gridW, gridH, 128.0 / w2, 128.0 / h2, true));
    if (isPVR)
    {
        ADD_TEST(BlitTest(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 0, 128, 128, "data/xorg_128x128_pvrtc4.raw",   false, gridW, gridH, 128.0 / w2, 128.0 / h2, true));
        ADD_TEST(BlitTest(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 0, 128, 128, "data/xorg_128x128_pvrtc2.raw",   false, gridW, gridH, 128.0 / w2, 128.0 / h2, true));
    }
    ADD_TEST(BlitTest(GL_ETC1_RGB8_OES,                    0, 128, 128, "data/xorg_128x128_etc1.raw",     false, gridW, gridH, 128.0 / w2, 128.0 / h2, true));
    ADD_TEST(ShaderBlitTest("mask", 128, 128, gridW, gridH * 0.5f, 128.0 / w2, 128.0 / h2));

    // Rotated small blended blits
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              127, 127, "data/xorg_127x127_rgba8888.raw", true, gridH, gridW, 127.0 / w2, 127.0 / h2, true));
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              128, 128, "data/xorg_128x128_rgba8888.raw", true, gridH, gridW, 128.0 / w2, 128.0 / h2, true));
    ADD_TEST(BlitTest(GL_RGB, GL_UNSIGNED_SHORT_5_6_5,        127, 127, "data/xorg_127x127_rgb565.raw",   true, gridH, gridW, 127.0 / w2, 127.0 / h2, true));
    ADD_TEST(BlitTest(GL_RGB, GL_UNSIGNED_SHORT_5_6_5,        128, 128, "data/xorg_128x128_rgb565.raw",   true, gridH, gridW, 128.0 / w2, 128.0 / h2, true));
    if (isPVR)
    {
        ADD_TEST(BlitTest(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 0, 128, 128, "data/xorg_128x128_pvrtc4.raw",   true, gridH, gridW, 128.0 / w2, 128.0 / h2, true));
        ADD_TEST(BlitTest(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 0, 128, 128, "data/xorg_128x128_pvrtc2.raw",   true, gridH, gridW, 128.0 / w2, 128.0 / h2, true));
    }
    ADD_TEST(BlitTest(GL_ETC1_RGB8_OES,                    0, 128, 128, "data/xorg_128x128_etc1.raw",     true, gridH, gridW, 128.0 / w2, 128.0 / h2, true));

    // Shader tests
    ADD_TEST(ShaderBlitTest("const", w, h));
    ADD_TEST(ShaderBlitTest("lingrad", w, h));
    ADD_TEST(ShaderBlitTest("radgrad", w, h));
    ADD_TEST(ShaderBlitTest("palette", w, h));
    ADD_TEST(ShaderBlitTest("blur", w, h));

    // CPU interleaving
    int wPOT = 1, hPOT = 1;
    while (wPOT < winWidth / 2)
      wPOT <<= 1;
    while (hPOT < winHeight / 2)
      hPOT <<= 1;

#if defined(SUPPORT_X11)
    ADD_TEST(CPUInterleavingTest(CPUI_XSHM_IMAGE, 2, 16, winWidth, winHeight));
    ADD_TEST(CPUInterleavingTest(CPUI_XSHM_IMAGE, 2, 32, winWidth, winHeight));
    ADD_TEST(CPUInterleavingTest(CPUI_XSHM_IMAGE, 2, 16, wPOT, hPOT));
    ADD_TEST(CPUInterleavingTest(CPUI_XSHM_IMAGE, 2, 32, wPOT, hPOT));
#endif

    ADD_TEST(CPUInterleavingTest(CPUI_TEXTURE_UPLOAD, 2, 16, winWidth, winHeight));
    ADD_TEST(CPUInterleavingTest(CPUI_TEXTURE_UPLOAD, 2, 32, winWidth, winHeight));
    ADD_TEST(CPUInterleavingTest(CPUI_TEXTURE_UPLOAD, 2, 16, wPOT, hPOT));
    ADD_TEST(CPUInterleavingTest(CPUI_TEXTURE_UPLOAD, 2, 32, wPOT, hPOT));

#if !defined(SUPPORT_ANDROID)
    ADD_TEST(CPUInterleavingTest(CPUI_EGL_LOCK_SURFACE, 2, 16, winWidth, winHeight));
    ADD_TEST(CPUInterleavingTest(CPUI_EGL_LOCK_SURFACE, 2, 32, winWidth, winHeight));
    ADD_TEST(CPUInterleavingTest(CPUI_EGL_LOCK_SURFACE, 2, 16, wPOT, hPOT));
    ADD_TEST(CPUInterleavingTest(CPUI_EGL_LOCK_SURFACE, 2, 32, wPOT, hPOT));
#endif
