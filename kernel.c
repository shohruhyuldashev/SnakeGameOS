#include "idt.h"
#include "io.h"
#include "acpi.h"

#define VGA ((volatile char*)0xB8000)
#define W 80
#define H 25

extern void acpi_init();
extern struct FADT* fadt;


extern unsigned char read_scancode();

/* ================= TIMER ================= */

volatile unsigned long ticks = 0;

void timer_handler(){
    ticks++;
}

void timer_init(){
    unsigned int freq = 100;
    unsigned int divisor = 1193180 / freq;

    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}

void sleep_ticks(unsigned long t){
    unsigned long start = ticks;
    while(ticks - start < t);
}

/* ================= SPEAKER ================= */

void speaker_on(){
    // PIT channel 2 square wave
    outb(0x43, 0xB6);

    // Set frequency (~440Hz)
    outb(0x42, 0xA9);
    outb(0x42, 0x04);

    // Enable speaker gate + data bits
    unsigned char tmp = inb(0x61);
    if((tmp & 3) != 3){
        outb(0x61, tmp | 3);
    }
}

void speaker_off(){
    unsigned char tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

void beep(){
    speaker_on();
    sleep_ticks(5);
    speaker_off();
}


void bios_style_beep(){
    speaker_on();
    sleep_ticks(10);
    speaker_off();

    sleep_ticks(10);

    speaker_on();
    sleep_ticks(10);
    speaker_off();
}




/* ================= VGA ================= */

void putc_xy(int x,int y,char c,char col){
    int p = (y*W + x) * 2;
    VGA[p] = c;
    VGA[p+1] = col;
}

void clear(){
    for(int y=0;y<H;y++)
        for(int x=0;x<W;x++)
            putc_xy(x,y,' ',0x0F);
}

void print_center(const char* s,int y){
    int len=0;
    while(s[len]) len++;
    int x=(W/2)-(len/2);
    for(int i=0;s[i];i++)
        putc_xy(x+i,y,s[i],0x0A);
}




void draw_help_bar(){
    // 1-qator
    const char* line1 =
        "CyberBro_OS  |  F2:Speed  F3:Restart  CTRL+SHIFT+DEL:Sysyem Restart(RealPC)";
    for(int i=0; line1[i]; i++)
        putc_xy(i, 0, line1[i], 0x1F);   // White on Blue

    // 2-qator
    const char* line2 =
        "WASD or Arrow Keys to move Snake";
    for(int i=0; line2[i]; i++)
        putc_xy(i, 1, line2[i], 0x0E);   // Yellow on black
}



/* ================= SNAKE GAME ================= */

int snake_x[128];
int snake_y[128];
int snake_len;

int dir_x, dir_y;
int food_x, food_y;

int speed_level = 2;
int speed_table[3] = {20, 10, 5};   // timer ticks per move

int game_running = 1;

void draw_border(){
    for(int x=0;x<W;x++){
        putc_xy(x,2,'#',0x02);
        putc_xy(x,H-1,'#',0x02);
    }
    for(int y=2;y<H;y++){
        putc_xy(0,y,'#',0x02);
        putc_xy(W-1,y,'#',0x02);
    }
}



void draw_food(){
    putc_xy(food_x,food_y,'*',0x0C);
}

void draw_snake(){
    for(int i=0;i<snake_len;i++)
        putc_xy(snake_x[i],snake_y[i], i? 'o':'O',0x0F);
}

void init_snake(){
    snake_len = 3;
    snake_x[0]=10; snake_y[0]=10;
    snake_x[1]=9;  snake_y[1]=10;
    snake_x[2]=8;  snake_y[2]=10;
    dir_x=1; dir_y=0;
    food_x=40; food_y=12;
    game_running = 1;
}

void game_over(){
    clear();
    draw_border();
    print_center("GAME OVER - Press F3 to Restart",12);
    speaker_on();
    sleep_ticks(30);
    speaker_off();
    bios_style_beep();
    game_running = 0;
    draw_help_bar();
}



void shutdown(){
    // Universal real hardware reset
    outb(0x64, 0xFE);
    for(;;)__asm__("hlt");
}


int ctrl_down = 0;
int shift_down = 0;
int e0_prefix = 0;


/* ================= INPUT ================= */

void input(){
    unsigned char sc = read_scancode();

    // ===== Extended scancode prefix =====
    if(sc == 0xE0){
        e0_prefix = 1;
        return;
    }

    // ===== CTRL press/release =====
    if(sc == 0x1D) ctrl_down = 1;
    if(sc == 0x9D) ctrl_down = 0;

    // ===== SHIFT press/release =====
    if(sc == 0x2A) shift_down = 1;
    if(sc == 0xAA) shift_down = 0;

    // ===== CTRL + SHIFT + DELETE =====
    // DELETE = E0 53
    if(e0_prefix && sc == 0x53){
        if(ctrl_down && shift_down){
            shutdown();
        }
    }

    // ===== WASD =====
    if(sc==0x11){dir_x=0;dir_y=-1;}
    if(sc==0x1F){dir_x=0;dir_y=1;}
    if(sc==0x1E){dir_x=-1;dir_y=0;}
    if(sc==0x20){dir_x=1;dir_y=0;}

    // ===== Arrow keys =====
    if(sc==0x48){dir_x=0;dir_y=-1;}
    if(sc==0x50){dir_x=0;dir_y=1;}
    if(sc==0x4B){dir_x=-1;dir_y=0;}
    if(sc==0x4D){dir_x=1;dir_y=0;}

    // ===== F2 speed =====
    if(sc==0x3C){
        speed_level++;
        if(speed_level>3) speed_level=1;
    }

    // ===== F3 restart =====
    if(sc==0x3D){
        clear();
	draw_help_bar();
        init_snake();
        draw_border();
        draw_food();
        draw_snake();
    }

    // reset prefix after handling
    e0_prefix = 0;
}

/* ================= GAME LOGIC ================= */

int collision(){
    if(snake_x[0] <= 0 || snake_x[0] >= W-1 ||
       snake_y[0] <= 2 || snake_y[0] >= H-1)
        return 1;
    return 0;
}

int food_eaten(){
    if(snake_x[0]==food_x && snake_y[0]==food_y){
	food_x = 2 + (snake_len*7)%(W-4);
	food_y = 3 + (snake_len*3)%(H-5);
        snake_len++;
        return 1;
    }
    return 0;
}

void move_snake(){
    for(int i=snake_len-1;i>0;i--){
        snake_x[i]=snake_x[i-1];
        snake_y[i]=snake_y[i-1];
    }
    snake_x[0]+=dir_x;
    snake_y[0]+=dir_y;
}

/* ================= KERNEL MAIN ================= */

void kernel_main(){

    idt_init();
    timer_init();

    clear();
    print_center("Cyber Snake OS created by CyberBro",12);
    sleep_ticks(150);   // ~3 seconds
    clear();
    print_center("Welcome to CyberSnake",12);
    sleep_ticks(100);
    clear();
    draw_help_bar();
    init_snake();
    draw_border();
    draw_food();
    draw_snake();

    int old_tail_x, old_tail_y;

    while(1){

        input();

        if(game_running){

            old_tail_x = snake_x[snake_len-1];
            old_tail_y = snake_y[snake_len-1];

            move_snake();

            if(collision()){
                game_over();
            }

            // erase old tail
            putc_xy(old_tail_x, old_tail_y,' ',0x0F);

            // draw new head
            putc_xy(snake_x[0],snake_y[0],'O',0x0F);

            if(food_eaten()){
                speaker_on();
                sleep_ticks(3);
                speaker_off();
                draw_food();
            }
        }

        sleep_ticks(speed_table[speed_level-1]);
    }
}
