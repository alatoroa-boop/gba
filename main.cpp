#include <cstdint>

// Registros de hardware de la GBA
#define REG_DISPCNT  *(volatile uint16_t*)0x04000000
#define REG_VCOUNT   *(volatile uint16_t*)0x04000006
#define REG_KEYINPUT *(volatile uint16_t*)0x04000130
#define VRAM         ((volatile uint16_t*)0x06000000)

// Máscaras de botones (activos en nivel bajo / 0 = presionado)
#define KEY_A      (1 << 0)
#define KEY_RIGHT  (1 << 4)
#define KEY_LEFT   (1 << 5)

// Conversión de color RGB a formato BGR555 de GBA
inline uint16_t RGB15(uint16_t r, uint16_t g, uint16_t b) {
    return r | (g << 5) | (b << 10);
}

// Sincronización vertical para evitar parpadeo (VBlank)
void vsync() {
    while (REG_VCOUNT >= 160);
    while (REG_VCOUNT < 160);
}

// Estructura del jugador
struct Player {
    int x, y;
    int vy;
    bool isGrounded;
    int width, height;

    void update(uint16_t keys) {
        // Movimiento lateral
        if (!(keys & KEY_LEFT))  x -= 1;
        if (!(keys & KEY_RIGHT)) x += 1;

        // Salto
        if (!(keys & KEY_A) && isGrounded) {
            vy = -5;
            isGrounded = false;
        }

        // Gravedad y física
        y += vy;
        if (!isGrounded) {
            vy += 1; // Fuerza de gravedad
        }

        // Suelo estático en Y = 120
        if (y >= 120) {
            y = 120;
            vy = 0;
            isGrounded = true;
        }

        // Límites de pantalla (GBA: 240x160)
        if (x < 0) x = 0;
        if (x > 240 - width) x = 240 - width;
    }

    void draw(uint16_t color) {
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                VRAM[(y + row) * 240 + (x + col)] = color;
            }
        }
    }
};

int main() {
    // Configura Modo 3 (Bitmap 240x160 a 15-bit color) y activa Fondo 2
    REG_DISPCNT = 0x0003 | 0x0400;

    Player mario = { 30, 120, 0, true, 8, 12 };
    uint16_t marioColor = RGB15(31, 0, 0); // Rojo clásico
    uint16_t bgColor = RGB15(15, 20, 31);    // Azul cielo

    // Limpia la pantalla inicial
    for (int i = 0; i < 240 * 160; ++i) {
        VRAM[i] = bgColor;
    }

    while (1) {
        vsync();

        // Borra la posición anterior dibujando el fondo
        mario.draw(bgColor);

        // Lee entrada y actualiza lógica
        uint16_t keys = REG_KEYINPUT;
        mario.update(keys);

        // Dibuja al personaje en su nueva posición
        mario.draw(marioColor);
    }

    return 0;
}
