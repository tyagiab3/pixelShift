/* This files provides address values that exist in the system */

#define SDRAM_BASE            0xC0000000
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_CHAR_BASE        0xC9000000

/* Cyclone V FPGA devices */    
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define TIMER_BASE            0xFF202000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030
#define PS2_BASE   			  0xFF200100

/* VGA colors */
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00
#define BLACK 0

#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

/* Constants for animation */
#define BOX_LEN 2
#define BOX_SIZE 10
#define NUM_BOXES 1
#define FALSE 0
#define TRUE 1
#define BG_COLOR WHITE

//Obstacle constants
#define OBSTACLE_SPACE 100
#define OBST_THICKNESS 20

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

// Begin part3.c code for Lab 7


volatile int pixel_buffer_start; // global variable
void draw_line(int x0, int x1, int y0, int y1, short int color);
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void wait_for_vsync();
void draw_box(int x, int y, short int line_color);
void clear_box(int x, int y, short int line_color);
void draw();
void updateLoctions();
void draw2();
void whiteBackground();
void blackBackground();
void gameOver();
void scoreClean();
void scoreTrack();
bool checkValidity();
void drawObstacle(int xObsPos, int yObsPos, int color);
void clearObstacle(int xObsPos, int yObsPos);
void updateObstacleLocation();
bool checkScoreAddition(int yObsPos);
void pushbutton_ISR();

void config_KEYs ();
void config_GIC ();
void enable_A9_interrupts ();

volatile int key_pressed = 0;

int xLocBox;
int yLocBox;
int prevXLocBox;
int prevYLocBox;
int colorBox;
int dy_box;
int dx_box;
int xObs1Pos;
int yObs1Pos;
int prevXObs1;
int prevYObs1;
int obs1Color;
int xObs2Pos;
int yObs2Pos;
int prevXObs2;
int prevYObs2;
int obs2Color;
bool start = TRUE;
int score = 0;
bool passedObs1 = false;
bool passedObs2 = false;
short int color[9] = {YELLOW, RED, GREEN, BLUE, CYAN, MAGENTA, GREY, PINK, ORANGE};
int OBST_SPEED = 1;

static const int spike[]  = {
    0x0000, 0x0000, 0x0000, 0x0000, 0xa514, 0xa514, 0x0000, 0x0000, 0x0000, 0x0000, 
    0x0000, 0xd69a, 0xce79, 0xad55, 0xa514, 0xa514, 0xad55, 0xce79, 0xce59, 0x0000, 
    0x0000, 0xd6ba, 0xce59, 0xad55, 0x73ae, 0x73ae, 0xad55, 0xce59, 0xbdf7, 0x0000, 
    0x0000, 0xbdf7, 0xa534, 0x528a, 0x4a69, 0x4a69, 0x528a, 0xad55, 0xbdf7, 0x0000, 
    0xa514, 0xa514, 0x73ae, 0x4a69, 0x4a69, 0x4a69, 0x4a69, 0x73ae, 0xa514, 0xa514, 
    0x94b2, 0xa514, 0x73ae, 0x4a69, 0x4a69, 0x4a69, 0x4a69, 0x73ae, 0xa514, 0xa514, 
    0x0000, 0x0000, 0xbdd7, 0x5aeb, 0x4a69, 0x4a69, 0x5aeb, 0xbdd7, 0x0000, 0x0000, 
    0x0000, 0xce59, 0xce59, 0xb5b6, 0x8430, 0x8430, 0xb5b6, 0xce79, 0xce59, 0x0000, 
    0x0000, 0xbdd7, 0xffff, 0xffff, 0xa514, 0xa514, 0xffff, 0xffff, 0xbdd7, 0x0000, 
    0x0000, 0x0000, 0x0000, 0x0000, 0x9cd3, 0x9cd3, 0x0000, 0x0000, 0x0000, 0x0000
};

void video_text(int x, int y, char * text_ptr) { 
	int offset;
	volatile char * character_buffer = (char *)FPGA_CHAR_BASE; // video character buffer
	offset = (y << 7) + x; 
	while (*(text_ptr)) {
		*(character_buffer + offset) =
		*(text_ptr); // write to the character buffer
		++text_ptr;
		++offset; 
	}
}



volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
//volatile int * SW_switch_ptr = (int *)SW_BASE; // SW slider switch address 
//volatile int * KEY_ptr = (int *)KEY_BASE; // pushbutton KEY address


void startScreen(){
//volatile int * KEY_ptr = (int *)KEY_BASE; // pushbutton KEY address
		//KEY_value = *(KEY_ptr); // read the pushbutton KEY
		// clear_screen(); 
	while (key_pressed == 0){
		scoreClean();
		char text_top_row[40] = "Pixel Shift";
		char text_bottom_row[50] = "Click Any Key To Start!";
		char* t1 = &text_top_row[0];
		char* t2 = &text_bottom_row[0];
    	/* update color */
		video_text(35, 29, t1);
		video_text(29, 30, t2);
		char text_bottom_row2[40] = "                              ";
		char* t3 = &text_bottom_row2[0];
		video_text(29, 31, t3);
	//	wait_for_vsync(); // swap front and back buffers on VGA vertical syn
	//	pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
	//	KEY_value = *(KEY_ptr); // read the pushbutton KEY values if (KEY_value != 0) // check if any KEY was pressed

		}

	start = FALSE;
	
	
}


  	

void endStartScreen(){
	
		start = FALSE;
		char text_top_row_clean[80] = "                                        ";
		char text_bottom_row_clean[50] = "                                             ";
    	char text_middle_row_clean[50] = "                         	";
		char* t1 = &text_top_row_clean[0];
		char* t2 = &text_bottom_row_clean[0];
		char* t3 = &text_middle_row_clean[0];
	/* update color */
		video_text(35, 29, t1);
		video_text(29, 30, t2);
	//video_text(29, 30, text_middle_row_clean);
	video_text(29, 31, t3);
	//	wait_for_vsync(); // swap front and back buffers on VGA vertical syn
	//	pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
		
	
}

/* Turn on interrupts in the ARM processor */
void enable_A9_interrupts(void){
	int status = 0b01010011; //Bit codes to unmask interrupts
	asm("msr cpsr, %[ps]" : : [ps]"r"(status)); // MSR instruction to write CPSR
}


void config_GIC(void){

*((int *) 0xFFFED848) = 0x00000101; //Timer and Key interrupts
*((int *) 0xFFFED108) = 0x00000300;

*((int *) 0xFFFEC104) = 0xFFFF; //Enable interrupts of all priorities

	
*((int *) 0xFFFEC100) = 1; // enable siginaling of interrupts; enable = 1

*((int *) 0xFFFED000) = 1; // enable = 1, (ICDDCR) send pending interrupts to CPUs
}

extern volatile int key_pressed;

void config_KEYs() {
	volatile int * KEY_ptr = (int *) KEY_BASE; 
	*(KEY_ptr + 2) = 0xF; // enable interrupts for all KEYs
}

//Checks which interrupt was caused (only checking for key currently)
void __attribute__ ((interrupt)) __cs3_isr_irq (void){
	// Read the ICCIAR from the processor interface
	int interruptID = *((int *) 0xFFFEC10C);

 	if (interruptID == 73) { //Check if keys casued interrupt
		pushbutton_ISR();	//Handler for key interrupt
	}
	
// Write to the End of Interrupt Register (ICCEOIR)
	*((int *) 0xFFFEC110) = interruptID; //Clean the interupt that was casued
	return;
}

void __attribute__ ((interrupt)) __cs3_isr_undef (void){
	while (1);
}

void __attribute__ ((interrupt)) __cs3_isr_swi (void){
	while (1);
}

void __attribute__ ((interrupt)) __cs3_isr_pabort (void){
	while (1);
}

void __attribute__ ((interrupt)) __cs3_isr_dabort (void){
	while (1);
}

void __attribute__ ((interrupt)) __cs3_isr_fiq (void){
	while (1);
}

//extern volatile int key_pressed;
void pushbutton_ISR(){
	volatile int * KEY_ptr = (int *) 0xFF200050;
	int keyClicked;
	keyClicked = *(KEY_ptr + 3); // read the pushbutton interrupt register
	*(KEY_ptr + 3) = keyClicked; // clear the interrupt
	
	if (keyClicked & 0x1){ // KEY0
		key_pressed = 1;
	}
	
	else if (keyClicked & 0x2){ // KEY1
		key_pressed = 2;
	}	
	
	else if (keyClicked & 0x4) // KEY2
		key_pressed = 4;
	
	else if (keyClicked & 0x8)// KEY3
	key_pressed = 8;
return;
}



int main(void)
{
	srand(time(NULL));
    volatile int * PS2_ptr = (int *)PS2_BASE;
	volatile int * SW_switch_ptr = (int *)SW_BASE; // SW slider switch address 
	volatile int * KEY_ptr = (int *)KEY_BASE; // pushbutton KEY address
	volatile int * HEX3_HEX0_ptr = (int *)HEX3_HEX0_BASE; 
	volatile int * HEX5_HEX4_ptr = (int *)HEX5_HEX4_BASE;
    // declare other variables(not shown)
	volatile int * pixel_ctrl_ptr = (int *)0xFF203020;;
    // initialize location and direction of rectangles(not shown)
	//int SW_value = 2;
//	int KEY_value = 0;
		config_GIC ();
		config_KEYs ();
		enable_A9_interrupts ();
		bool ran = FALSE;
		int z = 0;
		xLocBox = (RESOLUTION_X - BOX_SIZE) / 2;
		yLocBox = (RESOLUTION_Y - BOX_SIZE) / 2;
		colorBox = BLACK;
	
	int PS2_data, RVALID;
	
	
	*(PS2_ptr) = 0xFF;
	xObs1Pos = rand()%(RESOLUTION_X - OBSTACLE_SPACE);
	yObs1Pos = 0;
	xObs2Pos = rand()%(RESOLUTION_X - OBSTACLE_SPACE);
	yObs2Pos = 0;
	obs1Color = color[rand()%9];
	obs2Color = color[rand()%9];
		
    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
   clear_screen(); // pixel_buffer_start points to the pixel buffer
 //   whiteBackground();
	/* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen(); // pixel_buffer_start points to the pixel buffer
   // whiteBackground();
	while (1)
    {
		PS2_data = *(PS2_ptr); // read the Data register in the PS/2 port
		RVALID = PS2_data & 0x8000; // extract the RVALID field
		int RAVAIL = ( PS2_data & 0xFFFF0000) >> 16;

		char byte1 = 0;
		if (RAVAIL > 0){
			byte1 = PS2_data & 0xFF;
            if(byte1 == 0x74){
            	key_pressed = 1;
            }
            if(byte1 == 0x6b){
            	key_pressed = 2;
            }
		}
		//SW_value = *(SW_switch_ptr);
		//OBST_SPEED *= SW_value;
		//whiteBackground();
		while(start == TRUE){
		startScreen();
		}
		//wait_for_vsync(); // swap front and back buffers on VGA vertical syn
		//pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
		
	if(ran == FALSE){
			endStartScreen();
			ran == TRUE;
			// wait_for_vsync(); // swap front and back buffers on VGA vertical syn
		//pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
	if (z == 0){
	  *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
 //  clear_screen(); // pixel_buffer_start points to the pixel buffer
    whiteBackground();
	/* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
   // clear_screen(); // pixel_buffer_start points to the pixel buffer
    whiteBackground();	
		z = 1;
	}
	}
		
		
        // Erase any boxes and lines that were drawn in the last iteration 
        draw();
        // code for drawing the boxes and lines (not shown)
        // code for updating the locations of boxes (not shown)
        wait_for_vsync(); // swap front and back buffers on VGA vertical syn
		pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
		
		draw2();
		
	
		if (key_pressed == 99){
			
			
		}
		if (key_pressed != 0){ // check if any KEY was pressed

  
			
			if (key_pressed == 4 || yLocBox >= 230 || yLocBox <= 10 || checkValidity() || byte1 == 0x75){ // key2
				byte1 = 0;
                draw();
				wait_for_vsync(); // swap front and back buffers on VGA vertical syn
		pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
				int i = 1000000;
				while(i != 0)
					i--;
				gameOver();
				wait_for_vsync(); // swap front and back buffers on VGA vertical syn
		pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
				while(1){
				if (*KEY_ptr != 0){
					
				break;	
				}
				}
				  *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
 //  clear_screen(); // pixel_buffer_start points to the pixel buffer
    whiteBackground();
	/* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
   // clear_screen(); // pixel_buffer_start points to the pixel buffer
    whiteBackground();	
					char text_top_row_clean[40] = "                            ";
				char text_middle_row_clean[100] = "                         	";
				char text_bottom_row_clean[100] = "                                                        ";
				char* t1 = &text_top_row_clean[0];
				char* t2 = &text_middle_row_clean[0];
				char* t3 = &text_bottom_row_clean[0];
			video_text(35, 29, t1);
			video_text(34, 30, t2);
				video_text(29, 31, t3);
				//	wait_for_vsync(); // swap front and back buffers on VGA vertical syn
	//	pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
				

			
			}

}
		
    }
}
// code for subroutines

void draw(){
	draw_box(xLocBox, yLocBox, colorBox);
	scoreTrack();
	drawObstacle(xObs1Pos, yObs1Pos, obs1Color);
	if(yObs1Pos > 130 || yObs2Pos != 0)
		drawObstacle(xObs2Pos, yObs2Pos, obs2Color);
	
}
void draw2(){
    clear_box(prevXLocBox , prevYLocBox, WHITE);
    drawObstacle(prevXObs1, prevYObs1, WHITE);
    drawObstacle(prevXObs2, prevYObs2, WHITE);
    updateLoctions();
    updateObstacleLocation();
}


void updateLoctions(){
	prevXLocBox = xLocBox;
	prevYLocBox = yLocBox;
	prevXObs1 = xObs1Pos;
	prevYObs1 = yObs1Pos;
	prevXObs2 = xObs2Pos;
	prevYObs2 = yObs2Pos;
	if (key_pressed == 1){ // key0
		//yObs1Pos += 10;
		//key_pressed = 9;

 if(xLocBox >= 320){
	  xLocBox = 0;
	  }
	  else{
		  key_pressed = 9;
    	xLocBox += 20;
}  
		if(yLocBox >= 51)
		yLocBox -= 50;
		else
			yLocBox -= 10;;
  }
	else{
		yLocBox += 2;
	}
	if (key_pressed == 2){ // key1
		//yObs1Pos += 10;
	    //key_pressed = 9;
	   if(xLocBox <= 0){
	    xLocBox = 320;
	   }
	   else{
			key_pressed = 9;
    		xLocBox -= 20;
		} 
		if(yLocBox >= 51)
		yLocBox -= 50;
		else
			yLocBox -= 10;
	}
	else{
		yLocBox += 2;
	}
	score++;
}

void wait_for_vsync() {
    volatile int * pixel_ctrl_ptr = (int *) 0xFF203020;
    register int status;
    *pixel_ctrl_ptr = 1;
    status = *(pixel_ctrl_ptr + 3);
    while ((status & 0x01) != 0) {
        status = *(pixel_ctrl_ptr + 3);
    }
}

void clear_screen() {
    int x;
    int y;
    for (x = 0; x < RESOLUTION_X; x++) {
        for (y = 0; y < RESOLUTION_Y; y++) {
            plot_pixel(x, y, 0);
        }
    }
}

void plot_pixel(int x, int y, short int line_color) {
    *(short int *) (pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void whiteBackground(){
	int x;
    int y;
    for (x = 0; x < RESOLUTION_X; x++) {
        for (y = 0; y < RESOLUTION_Y; y++) {
            plot_pixel(x, y, BG_COLOR);
        }
    }
	int i , j;
	for(i = 0; i < 25; i++){
		for(j = RESOLUTION_Y - 10; j < RESOLUTION_Y; j++){
			plot_pixel(i, j, GREY);
		}
	}
}


void blackBackground(){
	int x;
    int y;
    for (x = 0; x < RESOLUTION_X; x++) {
        for (y = 0; y < RESOLUTION_Y; y++) {
            plot_pixel(x, y, YELLOW);
        }
    }
}
void gameOver(){
		clear_screen();
		char c = ' '; 
        char* scoreChar = &c;
		sprintf(scoreChar, "%i", score);
		char text_middle_row[40] = "Final Score:";
		strcat(text_middle_row, scoreChar);
		char text_top_row[40] = "GAME OVER !";
		char text_bottom_row[50] = "PRESS ANY KEY TO RESTART";
		char* t1 = &text_middle_row[0];
		char* t2 = &text_top_row[0];
		char* t3 = &text_bottom_row[0];
    	/* update color */
		video_text(35, 29, t2);
		video_text(34, 30, t1);
		video_text(29, 31, t3);
		scoreClean();
	
		xLocBox = (RESOLUTION_X - BOX_SIZE) / 2;
		yLocBox = (RESOLUTION_Y - BOX_SIZE) / 2;
		colorBox = BLACK;
	
		xObs1Pos = rand()%(RESOLUTION_X - OBSTACLE_SPACE);
		yObs1Pos = 0;
		xObs2Pos = rand()%(RESOLUTION_X - OBSTACLE_SPACE);
		yObs2Pos = 0;
}

void drawObstacle(int xObsPos, int yObsPos, int color){
	int i, j, k, l;
	for(i = 0; i < xObsPos; i++){
		for(j = yObsPos; j < yObsPos + OBST_THICKNESS; j++){
			plot_pixel(i, j, color);
		}
	}
	for(k = xObsPos + OBSTACLE_SPACE; k < RESOLUTION_X; k++){
		for(l = yObsPos; l < yObsPos + OBST_THICKNESS; l++){
			plot_pixel(k, l, color);
		}
	}
}
void clearObstacle(int xObsPos, int yObsPos){
	int i, j, k, l;
	for(i = 0; i < RESOLUTION_X; i++){
		for(j = yObsPos; j < yObsPos + OBST_THICKNESS; j++){
			plot_pixel(i, j, WHITE);
		}
	}
}
void updateObstacleLocation(){
	int i = 10000000;
    while(i != 0)
    	i--;
		if((yObs1Pos + OBST_THICKNESS) >= 210){
			xObs1Pos = rand()%(RESOLUTION_X - OBSTACLE_SPACE);
			if(xObs1Pos <= 40)
				xObs1Pos += 80;
			else if(xObs1Pos >= 280)
				xObs1Pos -= 80;
			yObs1Pos = 0;
			obs1Color = color[rand()%9];
		}else{
			yObs1Pos += OBST_SPEED;
		}
		
		if(yObs1Pos > 130 || yObs2Pos != 0){
			if((yObs2Pos + OBST_THICKNESS) >= 210){
				xObs2Pos = rand()%(RESOLUTION_X - OBSTACLE_SPACE);
				if(xObs2Pos <= 40)
					xObs2Pos += 80;
				else if(xObs2Pos >= 280)
					xObs2Pos -= 80;
				yObs2Pos = 0;
				obs2Color = color[rand()%9];
			}else{
				yObs2Pos += OBST_SPEED;
			}
	}
}



void scoreTrack(){
	char c = ' '; 
	char text_top[40] = "SCORE:";
    char* t1 = &c;
	sprintf(t1, "%i", score);
	char* t2 = &text_top[0];
    /* update color */
	video_text(0, 58,t2);
	video_text(0, 59, t1);
}

void scoreClean(){
	score = 0;
	char text_top[40] = "         ";
	char c[40] = "                                  ";
	char* t1 = &text_top[0];
	char* t2 = &c[0];
	video_text(0, 58, t1);
	video_text(0, 59, t2);
	
}

bool checkValidity(){
	int i, j;
	for(i = xLocBox; i < xLocBox + BOX_SIZE; i++){
		for(j = yLocBox; j < yLocBox + BOX_SIZE; j++){
			if((i <= xObs1Pos || i >= xObs1Pos+OBSTACLE_SPACE) && (j >= yObs1Pos && j <= yObs1Pos + OBST_THICKNESS)){
				return true;
			}
			if((i <= xObs2Pos || i >= xObs2Pos+OBSTACLE_SPACE) && (j >= yObs2Pos && j <= yObs2Pos + OBST_THICKNESS)){
				return true;
			}
		}
	}
	return false;
}

void clear_box(int x, int y, short int line_color){
	int i, j;
    for (i = x; i<x+BOX_SIZE; i++){
        for (j = y; j<y+BOX_SIZE; j++){
            if (spike[(j-y) * BOX_SIZE + (i-x)] != 0)
                plot_pixel( i,  j, line_color);
        }
    }

}

void draw_box(int x, int y, short int line_color){
	int i, j;
    for (i = x; i<x+BOX_SIZE; i++){
        for (j = y; j<y+BOX_SIZE; j++){
            if (spike[(j-y) * BOX_SIZE + (i-x)] != 0)
                plot_pixel( i,  j, spike[(j-y) * BOX_SIZE + (i-x)]);
        }
    }

}
