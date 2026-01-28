// WolfTech OS Kernel - Enterprise Edition
// Features: ELF binary loading, syscall translation, driver framework

#include <stdint.h>
#include <stddef.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

#define FB_WIDTH 1024
#define FB_HEIGHT 768
#define FB_BPP 32

// Framebuffer info
typedef struct {
    uint32_t *address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
} framebuffer_t;

framebuffer_t fb;

// ELF file structures for Linux binary compatibility
#define ELF_MAGIC 0x464C457F  // "\x7fELF"

typedef struct {
    uint32_t magic;
    uint8_t  bits;          // 1=32bit, 2=64bit
    uint8_t  endian;
    uint8_t  version;
    uint8_t  abi;
    uint8_t  padding[8];
    uint16_t type;
    uint16_t machine;
    uint32_t version2;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} __attribute__((packed)) elf_header_t;

typedef struct {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
} __attribute__((packed)) elf_program_header_t;

// Process structure
typedef struct {
    uint32_t pid;
    uint32_t entry_point;
    uint32_t *page_directory;
    char name[64];
    uint32_t state;
} process_t;

#define MAX_PROCESSES 64
process_t processes[MAX_PROCESSES];
uint32_t process_count = 0;

// Port I/O
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "d"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "d"(port));
    return ret;
}

// VGA functions
void vga_putchar(char c, int x, int y, uint8_t color) {
    uint16_t *vga = (uint16_t *)VGA_MEMORY;
    vga[y * VGA_WIDTH + x] = (color << 8) | c;
}

void vga_print(const char *str) {
    static int x = 0, y = 0;
    uint16_t *vga = (uint16_t *)VGA_MEMORY;
    
    while (*str) {
        if (*str == '\n') {
            x = 0;
            y++;
        } else {
            vga[y * VGA_WIDTH + x] = (0x0F << 8) | *str;
            x++;
            if (x >= VGA_WIDTH) {
                x = 0;
                y++;
            }
        }
        if (y >= VGA_HEIGHT) {
            y = 0;
        }
        str++;
    }
}

void vga_clear() {
    uint16_t *vga = (uint16_t *)VGA_MEMORY;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = (0x00 << 8) | ' ';
    }
}

// String functions
int strlen(const char *str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

void *memset(void *dest, int val, size_t len) {
    uint8_t *d = (uint8_t *)dest;
    while (len--) *d++ = val;
    return dest;
}

void *memcpy(void *dest, const void *src, size_t len) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (len--) *d++ = *s++;
    return dest;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(uint8_t *)s1 - *(uint8_t *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(uint8_t *)s1 - *(uint8_t *)s2;
}

// Memory management
uint8_t memory_heap[4 * 1024 * 1024]; // 4MB heap
uint32_t heap_used = 0;

void *kmalloc(size_t size) {
    if (heap_used + size > sizeof(memory_heap)) {
        return NULL;
    }
    void *ptr = &memory_heap[heap_used];
    heap_used += size;
    // Align to 16 bytes
    heap_used = (heap_used + 15) & ~15;
    return ptr;
}

void kfree(void *ptr) {
    (void)ptr;
    // Simple allocator - no free implementation
}

// Framebuffer
int init_framebuffer() {
    fb.address = (uint32_t *)0xFD000000;
    fb.width = FB_WIDTH;
    fb.height = FB_HEIGHT;
    fb.pitch = FB_WIDTH * 4;
    fb.bpp = FB_BPP;
    return 1;
}

void fb_putpixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= (int)fb.width || y < 0 || y >= (int)fb.height) return;
    fb.address[y * fb.width + x] = color;
}

void fb_fillrect(int x, int y, int w, int h, uint32_t color) {
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            fb_putpixel(x + dx, y + dy, color);
        }
    }
}

void fb_clear(uint32_t color) {
    for (uint32_t i = 0; i < fb.width * fb.height; i++) {
        fb.address[i] = color;
    }
}

// Timer
volatile uint32_t timer_ticks = 0;
void timer_handler() {
    timer_ticks++;
}

void init_timer(uint32_t freq) {
    uint32_t divisor = 1193180 / freq;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

// Keyboard
uint8_t keyboard_buffer[256];
int keyboard_head = 0;
int keyboard_tail = 0;

const uint8_t scancode_to_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

void keyboard_handler() {
    uint8_t scancode = inb(0x60);
    if (scancode < 128) {
        uint8_t ascii = scancode_to_ascii[scancode];
        if (ascii != 0) {
            keyboard_buffer[keyboard_head] = ascii;
            keyboard_head = (keyboard_head + 1) % 256;
        }
    }
}

// IDT
typedef struct {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

idt_entry_t idt[256];
idt_ptr_t idt_ptr;

void idt_set_gate(int num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

extern void isr_timer();
extern void isr_keyboard();
extern void isr_syscall();

void init_idt() {
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint32_t)&idt;

    memset(&idt, 0, sizeof(idt));

    // Hardware interrupts
    idt_set_gate(32, (uint32_t)isr_timer, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)isr_keyboard, 0x08, 0x8E);
    
    // System call interrupt (int 0x80 - Linux compatible!)
    idt_set_gate(0x80, (uint32_t)isr_syscall, 0x08, 0xEE);

    __asm__ __volatile__ ("lidt %0" : : "m"(idt_ptr));
    __asm__ __volatile__ ("sti");

    // Remap PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

// ELF Binary Loader
int load_elf_binary(void *elf_data, size_t size) {
    elf_header_t *hdr = (elf_header_t *)elf_data;
    
    // Verify ELF magic
    if (hdr->magic != ELF_MAGIC) {
        vga_print("Not a valid ELF file\n");
        return -1;
    }
    
    // Only support 32-bit for now
    if (hdr->bits != 1) {
        vga_print("Only 32-bit ELF supported\n");
        return -1;
    }
    
    vga_print("Loading ELF binary...\n");
    
    // Load program headers
    elf_program_header_t *phdr = (elf_program_header_t *)((uint8_t *)elf_data + hdr->phoff);
    
    for (int i = 0; i < hdr->phnum; i++) {
        if (phdr[i].type == 1) { // PT_LOAD
            void *dest = (void *)phdr[i].vaddr;
            void *src = (uint8_t *)elf_data + phdr[i].offset;
            
            memcpy(dest, src, phdr[i].filesz);
            
            // Zero out BSS
            if (phdr[i].memsz > phdr[i].filesz) {
                memset((uint8_t *)dest + phdr[i].filesz, 0, 
                       phdr[i].memsz - phdr[i].filesz);
            }
        }
    }
    
    // Create process entry
    if (process_count < MAX_PROCESSES) {
        processes[process_count].pid = process_count + 1;
        processes[process_count].entry_point = hdr->entry;
        processes[process_count].state = 1; // Running
        process_count++;
        
        vga_print("Binary loaded successfully\n");
        return 0;
    }
    
    return -1;
}

// Linux syscall compatibility layer
typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp;
} syscall_regs_t;

uint32_t handle_syscall(syscall_regs_t *regs) {
    uint32_t syscall_num = regs->eax;
    
    switch (syscall_num) {
        case 1:  // sys_exit
            vga_print("Process exiting\n");
            return 0;
            
        case 4:  // sys_write
            // regs->ebx = fd, regs->ecx = buf, regs->edx = count
            if (regs->ebx == 1 || regs->ebx == 2) { // stdout/stderr
                char *buf = (char *)regs->ecx;
                for (uint32_t i = 0; i < regs->edx; i++) {
                    if (buf[i] == '\n') vga_print("\n");
                    else {
                        char tmp[2] = {buf[i], 0};
                        vga_print(tmp);
                    }
                }
                return regs->edx;
            }
            return -1;
            
        case 45: // sys_brk (memory allocation)
            return (uint32_t)kmalloc(4096);
            
        default:
            vga_print("Unsupported syscall\n");
            return -1;
    }
}

// Driver framework
typedef struct {
    char name[32];
    int (*init)(void);
    int (*read)(void *buf, size_t len);
    int (*write)(const void *buf, size_t len);
} driver_t;

#define MAX_DRIVERS 32
driver_t drivers[MAX_DRIVERS];
int driver_count = 0;

int register_driver(const char *name, int (*init)(void), 
                   int (*read)(void *, size_t), 
                   int (*write)(const void *, size_t)) {
    if (driver_count >= MAX_DRIVERS) return -1;
    
    driver_t *drv = &drivers[driver_count++];
    
    int i = 0;
    while (name[i] && i < 31) {
        drv->name[i] = name[i];
        i++;
    }
    drv->name[i] = 0;
    
    drv->init = init;
    drv->read = read;
    drv->write = write;
    
    if (drv->init) {
        return drv->init();
    }
    
    return 0;
}

// Kernel main
void kernel_main() {
    vga_clear();
    vga_print("WolfTech OS Enterprise - Kernel Starting...\n");
    vga_print("===========================================\n\n");
    
    vga_print("Initializing IDT...\n");
    init_idt();
    
    vga_print("Initializing Timer...\n");
    init_timer(100);
    
    vga_print("Initializing Framebuffer...\n");
    init_framebuffer();
    
    vga_print("Driver framework ready\n");
    vga_print("ELF binary loader ready\n");
    vga_print("Linux syscall compatibility enabled\n");
    
    vga_print("\nStarting WolfTech Enterprise Services...\n");
    
    // Small delay
    for (volatile int i = 0; i < 10000000; i++);
    
    // Call server initialization
    extern void server_main();
    server_main();
    
    vga_print("System halted.\n");
    while (1) {
        __asm__ __volatile__ ("hlt");
    }
}
