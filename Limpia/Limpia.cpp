// tereré v1.0.0
// Autor: Yuri Alexander Pagel Krüger
// © 2026 Yuri Alexander Pagel Krüger. Todos los derechos reservados.

// 1. INCLUIR WINDOWS (Apagando TODO lo que choca con Raylib)
#define WIN32_LEAN_AND_MEAN
#define NOGDI   
#define NOUSER  
#include <windows.h>

// 2. EXTRAER SOLO EL COMANDO DE MODO OSCURO (Sin cargar la librería entera que da errores)
extern "C" HRESULT WINAPI DwmSetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
#pragma comment(lib, "dwmapi.lib")

// 3. LIBRERÍAS NORMALES Y RAYLIB
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <chrono>
#include "raylib.h"

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

std::atomic<int> current_step(0);
std::atomic<bool> is_processing(false);

struct Particle {
    Vector2 position;
    float speed;
    float size;
    float alpha;
};

// --- FUNCIÓN PARA DETECTAR EL MODO OSCURO DE TU PC ---
bool IsWindowsDarkMode() {
    DWORD value = 0;
    DWORD dataSize = sizeof(value);
    HKEY hKey;

    // Abrimos el registro de Windows donde se guarda tu personalización
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "AppsUseLightTheme", NULL, NULL, (LPBYTE)&value, &dataSize);
        RegCloseKey(hKey);
    }
    // Si el valor es 0, significa que tu Windows está en Modo Oscuro
    return value == 0;
}

// --- FUNCIÓN PARA PINTAR LA BARRA SUPERIOR ---
void ApplyTitleBarTheme() {
    HWND hwnd = (HWND)GetWindowHandle(); // Obtenemos la ventana de Raylib
    int useDarkMode = IsWindowsDarkMode() ? 1 : 0;

    // 20 es el código interno de Windows para activar el modo oscuro inmersivo
    DwmSetWindowAttribute(hwnd, 20, &useDarkMode, sizeof(useDarkMode));
}

void ExecuteBackgroundProcess() {
    is_processing = true;

    current_step = 1;
    system("del /s /f /q %temp%\\*.* >nul 2>&1");
    system("del /s /f /q C:\\Windows\\temp\\*.* >nul 2>&1");
    system("PowerShell.exe -NoProfile -Command Clear-RecycleBin -Confirm:$false >nul 2>&1");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    current_step = 2;
    system("netsh winsock reset >nul 2>&1");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    current_step = 3;
    system("ipconfig /flushdns >nul 2>&1");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    current_step = 4;
    system("sfc /scannow");

    current_step = 0;
    is_processing = false;
}

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "tereré");

    // ==========================================
    // APLICAR TEMA OSCURO/CLARO DE WINDOWS
    ApplyTitleBarTheme();
    // ==========================================

    Image icon = LoadImage("terere.png");
    SetWindowIcon(icon);
    UnloadImage(icon);

    SetTargetFPS(60);

    Color pinkColor = { 218, 24, 132, 255 };
    Color blackBg = { 15, 15, 15, 255 };

    Font roboto = LoadFontEx("Roboto-Regular.ttf", 64, 0, 250);
    SetTextureFilter(roboto.texture, TEXTURE_FILTER_BILINEAR);

    HideCursor();

    std::vector<Particle> particles;
    for (int i = 0; i < 100; i++) {
        particles.push_back({
            {(float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight)},
            (float)GetRandomValue(10, 50) / 10.0f,
            (float)GetRandomValue(1, 3),
            (float)GetRandomValue(50, 200) / 255.0f
            });
    }

    std::string carousel_texts[5] = {
        "",
        "Estoy haciendo limpieza de archivos temporales",
        "Reiniciar tu conexion de red para dejarla como nueva",
        "Limpio el cache de tu navegacion",
        "Compruebo que todo funcione bien (esto puede tardar)"
    };

    while (!WindowShouldClose()) {
        for (auto& p : particles) {
            p.position.y += p.speed;
            if (p.position.y > screenHeight) {
                p.position.y = 0;
                p.position.x = (float)GetRandomValue(0, screenWidth);
            }
        }

        Vector2 mousePos = GetMousePosition();

        Rectangle btnIniciar = { screenWidth / 2.0f - 100, screenHeight / 2.0f - 40, 200, 50 };
        Rectangle btnSalir = { screenWidth / 2.0f - 100, screenHeight / 2.0f + 30, 200, 50 };

        if (!is_processing) {
            if (CheckCollisionPointRec(mousePos, btnIniciar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                std::thread backgroundThread(ExecuteBackgroundProcess);
                backgroundThread.detach();
            }
            if (CheckCollisionPointRec(mousePos, btnSalir) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                break;
            }
        }

        BeginDrawing();
        ClearBackground(blackBg);

        for (auto& p : particles) {
            DrawCircleV(p.position, p.size, Fade(WHITE, p.alpha));
        }

        if (!is_processing) {

            DrawTextEx(roboto, "tereré", { screenWidth / 2.0f - MeasureTextEx(roboto, "tereré", 60, 2).x / 2.0f, 150 }, 60, 2, pinkColor);

            bool hoverIniciar = CheckCollisionPointRec(mousePos, btnIniciar);
            DrawRectangleRounded(btnIniciar, 0.2f, 10, hoverIniciar ? pinkColor : Fade(pinkColor, 0.7f));
            Vector2 textIniciarSize = MeasureTextEx(roboto, "Iniciar", 30, 1);
            float textIniciarX = btnIniciar.x + (btnIniciar.width - textIniciarSize.x) / 2.0f;
            float textIniciarY = btnIniciar.y + (btnIniciar.height - textIniciarSize.y) / 2.0f;
            DrawTextEx(roboto, "Iniciar", { textIniciarX, textIniciarY }, 30, 1, WHITE);

            bool hoverSalir = CheckCollisionPointRec(mousePos, btnSalir);
            DrawRectangleRounded(btnSalir, 0.2f, 10, hoverSalir ? pinkColor : Fade(pinkColor, 0.7f));
            Vector2 textSalirSize = MeasureTextEx(roboto, "Salir", 30, 1);
            float textSalirX = btnSalir.x + (btnSalir.width - textSalirSize.x) / 2.0f;
            float textSalirY = btnSalir.y + (btnSalir.height - textSalirSize.y) / 2.0f;
            DrawTextEx(roboto, "Salir", { textSalirX, textSalirY }, 30, 1, WHITE);

            DrawTextEx(roboto, "Yuri Alexander Pagel Krüger", { screenWidth / 2.0f - MeasureTextEx(roboto, "Yuri Alexander Pagel Krüger", 20, 1).x / 2.0f, screenHeight - 50 }, 20, 1, GRAY);

        }
        else {
            DrawRing({ screenWidth / 2.0f, screenHeight / 2.0f - 50 }, 40, 50, (float)(GetTime() * 100), (float)(GetTime() * 100) + 250, 64, pinkColor);

            std::string text = carousel_texts[current_step];
            Vector2 textSize = MeasureTextEx(roboto, text.c_str(), 24, 1);
            DrawTextEx(roboto, text.c_str(), { screenWidth / 2.0f - textSize.x / 2.0f, screenHeight / 2.0f + 60 }, 24, 1, WHITE);
        }

        DrawCircleGradient((int)mousePos.x, (int)mousePos.y, 25, Fade(BLACK, 0.6f), Fade(BLACK, 0.0f));
        DrawCircleV(mousePos, 15, Fade(pinkColor, 0.3f));
        DrawCircleV(mousePos, 5, pinkColor);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
