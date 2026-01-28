// WolfTech OS Desktop Environment
// With Wallpaper Engine and Enterprise Dashboard Integration

#include <stdint.h>
#include <stddef.h>

// External functions
extern void fb_putpixel(int x, int y, uint32_t color);
extern void fb_fillrect(int x, int y, int w, int h, uint32_t color);
extern void fb_clear(uint32_t color);
extern void *kmalloc(size_t size);
extern void kfree(void *ptr);
extern int strlen(const char *str);
extern void *memset(void *dest, int val, size_t len);
extern void *memcpy(void *dest, const void *src, size_t len);
extern volatile uint32_t timer_ticks;
extern int strcmp(const char *s1, const char *s2);

typedef struct {
    uint32_t *address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
} framebuffer_t;

extern framebuffer_t fb;
extern uint32_t employee_count;

// Colors - WolfTech Corporate Theme
#define COLOR_BG        0xFF0a0e1a
#define COLOR_PANEL     0xFF1a1f35
#define COLOR_WINDOW    0xFF1e2330
#define COLOR_BORDER    0xFF2a3f5f
#define COLOR_ACCENT    0xFF00d4aa
#define COLOR_TEXT      0xFFe0e0e0
#define COLOR_SUCCESS   0xFF00ff88
#define COLOR_WARNING   0xFFffaa00
#define COLOR_ERROR     0xFFff4444
#define COLOR_WOLF_BLUE 0xFF0066ff
#define COLOR_WOLF_CYAN 0xFF00ccff

#define PANEL_HEIGHT 32
#define TITLE_HEIGHT 24
#define BORDER_WIDTH 2
#define MAX_WINDOWS 16

// Wallpaper types
typedef enum {
    WALLPAPER_STATIC,
    WALLPAPER_GRADIENT,
    WALLPAPER_ANIMATED_WAVE,
    WALLPAPER_PARTICLES,
    WALLPAPER_MATRIX
} wallpaper_type_t;

typedef struct {
    wallpaper_type_t type;
    uint32_t color1;
    uint32_t color2;
    uint32_t animation_speed;
    uint8_t enabled;
} wallpaper_config_t;

wallpaper_config_t wallpaper = {
    .type = WALLPAPER_ANIMATED_WAVE,
    .color1 = COLOR_BG,
    .color2 = COLOR_WOLF_BLUE,
    .animation_speed = 1,
    .enabled = 1
};

// Particle system for wallpaper
typedef struct {
    float x, y;
    float vx, vy;
    uint32_t color;
    uint8_t active;
} particle_t;

#define MAX_PARTICLES 100
particle_t particles[MAX_PARTICLES];

// Simple font (8x8)
const uint8_t font_8x8[128][8] = {
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['0'] = {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00},
    ['1'] = {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00},
    ['2'] = {0x3C, 0x66, 0x06, 0x0C, 0x18, 0x30, 0x7E, 0x00},
    ['3'] = {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00},
    ['4'] = {0x0C, 0x1C, 0x3C, 0x6C, 0x7E, 0x0C, 0x0C, 0x00},
    ['5'] = {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00},
    ['6'] = {0x3C, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00},
    ['7'] = {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00},
    ['8'] = {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00},
    ['9'] = {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00},
    [':'] = {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00},
    ['%'] = {0x62, 0x64, 0x08, 0x10, 0x20, 0x4C, 0x8C, 0x00},
    ['('] = {0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00},
    [')'] = {0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00},
    ['-'] = {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00},
    ['.'] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},
    ['/'] = {0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00},
    ['A'] = {0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    ['B'] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    ['C'] = {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00},
    ['D'] = {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00},
    ['E'] = {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00},
    ['F'] = {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00},
    ['G'] = {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00},
    ['H'] = {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    ['I'] = {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00},
    ['L'] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00},
    ['M'] = {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00},
    ['N'] = {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00},
    ['O'] = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    ['P'] = {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00},
    ['R'] = {0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66, 0x00},
    ['S'] = {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00},
    ['T'] = {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    ['U'] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    ['V'] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
    ['W'] = {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},
    ['Y'] = {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00},
    ['a'] = {0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00},
    ['b'] = {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00},
    ['c'] = {0x00, 0x00, 0x3C, 0x66, 0x60, 0x66, 0x3C, 0x00},
    ['d'] = {0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00},
    ['e'] = {0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00},
    ['f'] = {0x1C, 0x30, 0x30, 0x7C, 0x30, 0x30, 0x30, 0x00},
    ['h'] = {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00},
    ['i'] = {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00},
    ['l'] = {0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
    ['m'] = {0x00, 0x00, 0x66, 0x7F, 0x7F, 0x6B, 0x63, 0x00},
    ['n'] = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00},
    ['o'] = {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00},
    ['p'] = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60},
    ['r'] = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60},
    ['s'] = {0x00, 0x00, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x00},
    ['t'] = {0x18, 0x18, 0x7E, 0x18, 0x18, 0x18, 0x0E, 0x00},
    ['u'] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00},
    ['v'] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
    ['w'] = {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x3E, 0x36, 0x00},
    ['y'] = {0x00, 0x00, 0x66, 0x66, 0x3E, 0x06, 0x3C, 0x00},
};

void draw_char(int x, int y, char c, uint32_t color) {
    if (c < 0 || c >= 128) return;
    
    for (int dy = 0; dy < 8; dy++) {
        uint8_t row = font_8x8[(int)c][dy];
        for (int dx = 0; dx < 8; dx++) {
            if (row & (1 << (7 - dx))) {
                fb_putpixel(x + dx, y + dy, color);
            }
        }
    }
}

void draw_string(int x, int y, const char *str, uint32_t color) {
    int cx = x;
    while (*str) {
        if (*str == '\n') {
            cx = x;
            y += 10;
        } else {
            draw_char(cx, y, *str, color);
            cx += 8;
        }
        str++;
    }
}

// Simple sine approximation (no floating point)
int fast_sin(int angle) {
    // Angle in degrees 0-360
    angle = angle % 360;
    if (angle < 0) angle += 360;
    
    // Simple lookup table
    int sin_table[91] = {
        0, 17, 35, 52, 70, 87, 105, 122, 139, 156, 174, 191, 208, 225, 242,
        259, 276, 292, 309, 326, 342, 358, 375, 391, 407, 423, 438, 454, 469,
        485, 500, 515, 530, 545, 559, 574, 588, 602, 616, 629, 643, 656, 669,
        682, 695, 707, 719, 731, 743, 755, 766, 777, 788, 799, 809, 819, 829,
        839, 848, 857, 866, 875, 883, 891, 899, 906, 914, 921, 927, 934, 940,
        946, 951, 956, 961, 966, 970, 974, 978, 982, 985, 988, 990, 993, 995,
        996, 998, 999, 999, 1000, 1000
    };
    
    if (angle <= 90) return sin_table[angle];
    if (angle <= 180) return sin_table[180 - angle];
    if (angle <= 270) return -sin_table[angle - 180];
    return -sin_table[360 - angle];
}

// Wallpaper rendering
void render_wallpaper_gradient() {
    for (uint32_t y = 0; y < fb.height; y++) {
        // Interpolate between color1 and color2
        uint32_t ratio = (y * 256) / fb.height;
        uint32_t inv_ratio = 256 - ratio;
        
        uint8_t r1 = (wallpaper.color1 >> 16) & 0xFF;
        uint8_t g1 = (wallpaper.color1 >> 8) & 0xFF;
        uint8_t b1 = wallpaper.color1 & 0xFF;
        
        uint8_t r2 = (wallpaper.color2 >> 16) & 0xFF;
        uint8_t g2 = (wallpaper.color2 >> 8) & 0xFF;
        uint8_t b2 = wallpaper.color2 & 0xFF;
        
        uint8_t r = (r1 * inv_ratio + r2 * ratio) / 256;
        uint8_t g = (g1 * inv_ratio + g2 * ratio) / 256;
        uint8_t b = (b1 * inv_ratio + b2 * ratio) / 256;
        
        uint32_t color = 0xFF000000 | (r << 16) | (g << 8) | b;
        
        for (uint32_t x = 0; x < fb.width; x++) {
            fb_putpixel(x, y, color);
        }
    }
}

void render_wallpaper_wave() {
    static uint32_t wave_offset = 0;
    wave_offset += wallpaper.animation_speed;
    
    for (uint32_t y = 0; y < fb.height; y++) {
        for (uint32_t x = 0; x < fb.width; x++) {
            // Create wave pattern
            int angle = ((x * 360) / fb.width + wave_offset) % 360;
            int wave = fast_sin(angle) / 20;
            
            int adjusted_y = y + wave;
            uint32_t ratio = (adjusted_y * 256) / fb.height;
            if (ratio > 255) ratio = 255;
            uint32_t inv_ratio = 256 - ratio;
            
            uint8_t r1 = (wallpaper.color1 >> 16) & 0xFF;
            uint8_t g1 = (wallpaper.color1 >> 8) & 0xFF;
            uint8_t b1 = wallpaper.color1 & 0xFF;
            
            uint8_t r2 = (wallpaper.color2 >> 16) & 0xFF;
            uint8_t g2 = (wallpaper.color2 >> 8) & 0xFF;
            uint8_t b2 = wallpaper.color2 & 0xFF;
            
            uint8_t r = (r1 * inv_ratio + r2 * ratio) / 256;
            uint8_t g = (g1 * inv_ratio + g2 * ratio) / 256;
            uint8_t b = (b1 * inv_ratio + b2 * ratio) / 256;
            
            uint32_t color = 0xFF000000 | (r << 16) | (g << 8) | b;
            fb_putpixel(x, y, color);
        }
    }
}

void init_particles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].x = (timer_ticks * 17 + i * 37) % fb.width;
        particles[i].y = (timer_ticks * 23 + i * 43) % fb.height;
        particles[i].vx = ((i % 5) - 2) * 0.5f;
        particles[i].vy = ((i % 7) - 3) * 0.5f;
        particles[i].color = COLOR_ACCENT;
        particles[i].active = 1;
    }
}

void update_particles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            
            // Wrap around
            if (particles[i].x < 0) particles[i].x = fb.width;
            if (particles[i].x >= fb.width) particles[i].x = 0;
            if (particles[i].y < 0) particles[i].y = fb.height;
            if (particles[i].y >= fb.height) particles[i].y = 0;
        }
    }
}

void render_particles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            int x = (int)particles[i].x;
            int y = (int)particles[i].y;
            
            // Draw particle with glow
            fb_putpixel(x, y, particles[i].color);
            fb_putpixel(x+1, y, particles[i].color & 0x80FFFFFF);
            fb_putpixel(x-1, y, particles[i].color & 0x80FFFFFF);
            fb_putpixel(x, y+1, particles[i].color & 0x80FFFFFF);
            fb_putpixel(x, y-1, particles[i].color & 0x80FFFFFF);
        }
    }
}

void render_wallpaper() {
    if (!wallpaper.enabled) {
        fb_clear(COLOR_BG);
        return;
    }
    
    switch (wallpaper.type) {
        case WALLPAPER_GRADIENT:
            render_wallpaper_gradient();
            break;
        case WALLPAPER_ANIMATED_WAVE:
            render_wallpaper_wave();
            break;
        case WALLPAPER_PARTICLES:
            fb_clear(COLOR_BG);
            update_particles();
            render_particles();
            break;
        default:
            fb_clear(COLOR_BG);
            break;
    }
}

// Window structure
typedef struct {
    int x, y, width, height;
    char title[64];
    uint32_t active;
    uint32_t visible;
    uint32_t window_type; // 0=normal, 1=dashboard, 2=employee
} Window;

Window windows[MAX_WINDOWS];
int window_count = 0;

void draw_panel() {
    fb_fillrect(0, 0, fb.width, PANEL_HEIGHT, COLOR_PANEL);
    
    // WolfTech logo
    draw_string(12, 12, "WOLFTECH", COLOR_ACCENT);
    
    // System info
    char info[64];
    uint32_t uptime_sec = timer_ticks / 100;
    uint32_t uptime_min = uptime_sec / 60;
    
    draw_string(fb.width - 200, 12, "Uptime: ", COLOR_TEXT);
    // Simple number rendering
    info[0] = '0' + (uptime_min / 10);
    info[1] = '0' + (uptime_min % 10);
    info[2] = 'm';
    info[3] = 0;
    draw_string(fb.width - 120, 12, info, COLOR_SUCCESS);
    
    // Employee count
    info[0] = 'E';
    info[1] = 'm';
    info[2] = 'p';
    info[3] = ':';
    info[4] = ' ';
    info[5] = '0' + (employee_count % 10);
    info[6] = 0;
    draw_string(fb.width - 60, 12, info, COLOR_TEXT);
}

int create_window(int x, int y, int width, int height, const char *title, uint32_t type) {
    if (window_count >= MAX_WINDOWS) return -1;
    
    int idx = window_count++;
    windows[idx].x = x;
    windows[idx].y = y;
    windows[idx].width = width;
    windows[idx].height = height;
    windows[idx].visible = 1;
    windows[idx].active = 0;
    windows[idx].window_type = type;
    
    int i = 0;
    while (title[i] && i < 63) {
        windows[idx].title[i] = title[i];
        i++;
    }
    windows[idx].title[i] = 0;
    
    return idx;
}

void draw_window(int idx) {
    if (idx < 0 || idx >= window_count || !windows[idx].visible) return;
    
    Window *w = &windows[idx];
    
    uint32_t border_color = w->active ? COLOR_ACCENT : COLOR_BORDER;
    
    // Border
    fb_fillrect(w->x - BORDER_WIDTH, w->y - TITLE_HEIGHT - BORDER_WIDTH,
                w->width + 2 * BORDER_WIDTH, BORDER_WIDTH, border_color);
    fb_fillrect(w->x - BORDER_WIDTH, w->y + w->height,
                w->width + 2 * BORDER_WIDTH, BORDER_WIDTH, border_color);
    fb_fillrect(w->x - BORDER_WIDTH, w->y - TITLE_HEIGHT,
                BORDER_WIDTH, w->height + TITLE_HEIGHT, border_color);
    fb_fillrect(w->x + w->width, w->y - TITLE_HEIGHT,
                BORDER_WIDTH, w->height + TITLE_HEIGHT, border_color);
    
    // Title bar
    fb_fillrect(w->x, w->y - TITLE_HEIGHT, w->width, TITLE_HEIGHT, COLOR_PANEL);
    draw_string(w->x + 8, w->y - TITLE_HEIGHT + 8, w->title, COLOR_TEXT);
    
    // Close button
    fb_fillrect(w->x + w->width - 20, w->y - TITLE_HEIGHT + 4, 16, 16, COLOR_ERROR);
    
    // Content
    fb_fillrect(w->x, w->y, w->width, w->height, COLOR_WINDOW);
}

void draw_dashboard_content(Window *w) {
    extern void draw_web_dashboard();
    // Simplified dashboard in window
    draw_string(w->x + 20, w->y + 20, "WOLFTECH ENTERPRISE DASHBOARD", COLOR_ACCENT);
    draw_string(w->x + 20, w->y + 50, "Status: ONLINE", COLOR_SUCCESS);
    draw_string(w->x + 20, w->y + 70, "Employees: 5", COLOR_TEXT);
    draw_string(w->x + 20, w->y + 90, "Sessions: Active", COLOR_TEXT);
    draw_string(w->x + 20, w->y + 110, "Services: Running", COLOR_SUCCESS);
}

void de_main() {
    // Initialize wallpaper
    init_particles();
    
    // Create windows
    int dash_win = create_window(50, 80 + PANEL_HEIGHT, 700, 500, 
                                  "Enterprise Dashboard", 1);
    int emp_win = create_window(200, 150 + PANEL_HEIGHT, 600, 400, 
                                 "Employee Management", 2);
    
    if (dash_win >= 0) windows[dash_win].active = 1;
    
    uint32_t last_tick = 0;
    uint32_t frame = 0;
    
    while (1) {
        if (timer_ticks - last_tick > 2) {
            // Render wallpaper
            render_wallpaper();
            
            // Draw panel
            draw_panel();
            
            // Draw windows
            for (int i = 0; i < window_count; i++) {
                draw_window(i);
                if (windows[i].window_type == 1) {
                    draw_dashboard_content(&windows[i]);
                }
            }
            
            last_tick = timer_ticks;
            frame++;
        }
        
        __asm__ __volatile__ ("hlt");
    }
}
