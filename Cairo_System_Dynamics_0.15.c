/*
 * Copyright (C)2017-2022 Nuno Ferreira
 *
 * Cairo System Dynamics is  free software; you can redistribute  it and/or
 * modify it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 3  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

/*---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <math.h>

/* Compile on Unix-like systems: gcc Cairo_System_Dynamics_0.15.c -lm -o Cairo_System_Dynamics -Wall -Wextra -O2 `pkg-config --cflags --libs gtk+-3.0` && ./Cairo_System_Dynamics */

void memory_delete();

/////    GLOBAL VARIABLES    ////////////////////////////////////////////////////////////////////////////////////////

GObject *drawing_area;
GObject *drawing_area2;
cairo_surface_t *plot_background;

struct 
{
    cairo_surface_t *image;
    cairo_surface_t *surface;
    gint IMAGE_WIDTH;
    gint IMAGE_HEIGHT;
    gboolean SIMULATION_PAUSE;
    gboolean INTERACTION;
} global_settings;

struct RGB
{
    float red,green,blue;  //this will describe the planet colour
};

enum state {HEALTHY, DOCTOR, INFECTED};
    
struct BODY
{
    int body_id;
    long double x, y, xtemp, ytemp, vx, vy, vx_temp, vy_temp, ax, ay, m;  //Variables of position, velocity, acceleration and mass
    struct RGB color; //Colour of the body (I honestly prefer Color as it better matches the phonetics)
    gboolean selected;
    gboolean delete;    
    /* Time at which the infection occured: This will be used to determine how long the creature will live after being infected */
    float time_infection;
    enum state status; /*this determines if the organism is healthy, doctor or infected */
};
   
typedef struct BODY Body; 
Body *body;

struct WALL
{
    int Wall_id;
    double x1, x2, y1, y2;  //Wall coordinates
    gboolean selected;
    gboolean delete;
};

typedef struct WALL Wall; 
Wall *wall;

/*Number of iterations */
int N_iteracao = 0;

/* Simulation time resolution in seconds */
double h;   

/* Number of people/organisms */
int N_bodies;    

/* Number of walls */
int N_walls;    

/* Number of healthy organisms */
int number_healthy = 0; 
int number_healthy_B = 0; 
/* Number of doctors or imunological agents */
int number_doctors = 0;
int number_doctors_B = 0;
/* Number of infected organisms */
int number_infected = 0; // ;
int number_infected_B = 0; 
/*tag that determines if the simulation has ended because all non-doctors have died */
gboolean SIMULATION_END_DEATH = TRUE;

/*tag that determines if the simulation has ended because no-one is infected */
gboolean SIMULATION_END_LIFE = FALSE;

/* Size of the bodies that has been stored in memory */
double SIZE_TEMP;  

/*Mouse click logical tags */
float x_right_mouse_click, y_right_mouse_click, x_right_mouse_click_temp, y_right_mouse_click_temp;






gboolean drawArrow;    
gboolean draw_Select_Rectangle;    
gboolean LEFT_MOUSE_PRESSED = FALSE;
gboolean RIGHT_MOUSE_PRESSED = FALSE;
gboolean SPACE_KEY_PRESSED = FALSE;
gboolean A_KEY_PRESSED = FALSE;
gboolean D_KEY_PRESSED = FALSE;



/////////////// INITIATE VARIABLES    //////////////////////////////////////////////////////////////////////////////////////////////////





static void init_vars()
{
    global_settings.IMAGE_WIDTH = 500;
    global_settings.IMAGE_HEIGHT = 350;  
    global_settings.surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, global_settings.IMAGE_WIDTH, global_settings.IMAGE_HEIGHT);    

    plot_background = cairo_image_surface_create_from_png("axis.png");

    global_settings.SIMULATION_PAUSE = FALSE; 
    global_settings.INTERACTION = TRUE;
    drawArrow = FALSE;
    draw_Select_Rectangle = FALSE;
    SIZE_TEMP = 10.0;
    N_bodies = 0;
    N_walls = 4;
    h = 0.01;
    
    /////   Dummy loop variables    //////////////
    register unsigned int n;
    register unsigned int l;
       
    body = (Body*)malloc(N_bodies * sizeof(Body));   /* allocates arrays that are the size of N_bodies: */
     
    for (n = 0; n < N_bodies; n++)   /* defines bodies attributes: */
    {
	    body[n].body_id = n+1;
        body[n].selected = FALSE;
        body[n].delete = FALSE;
        body[n].status = HEALTHY;
        body[n].x = -50;
	    body[n].y = -50;
	    body[n].xtemp = -50;
	    body[n].ytemp = -50;
	    body[n].vx = 0;
	    body[n].vy = 0;
	    body[n].ax = 0;
	    body[n].ay = 0;
	    body[n].m = SIZE_TEMP;	
	    body[n].color.red = 0;
	    body[n].color.green = 0;  
	    body[n].color.blue = 0;     
    }  
  
    /* allocates the boundary array */
    wall = (Wall*)malloc(N_walls * sizeof(Wall));   
     
    for (n = 0; n < N_walls; n++)   /* defines bodies attributes: */
    {
	    wall[n].Wall_id = n+1;
        wall[n].selected = FALSE;
        wall[n].delete = FALSE;
    } 
    
}

///////////////    PHYSICS CALCULATIONS    //////////////////////////////////////////////////////////////////////////////////////////////


void main_loop(void) {

    N_iteracao++;
    
    double r;   /* distance between bodies */
    /* Loop variables are declared in the register to make code faster */
    register int n;
    register int l;

    for (n = 0; n < N_bodies; n++){
        body[n].x = body[n].x + body[n].vx*h;  /* Update positions of the CB's */
        body[n].y = body[n].y + body[n].vy*h;
    }

    ///////////////   Calculate accelerations of the CB's   ////////////

    for (n = 0; n < N_bodies; n++){
        for (l = 0; l < N_bodies; l++){
            body[n].ax = 0;
            body[n].ay = 0;	
        }
    }

    //////////////   Calculate velocities of the CB's   ////////////////
    for (n = 0; n < N_bodies; n++){
               
    ////////////////   Check for collisions with other bodies   ////////  
    for (l = 0; l < N_bodies; l++){
        if (n > l){
            r = (pow((body[n].x-body[l].x),2) + pow((body[n].y-body[l].y),2));   //  Calculate distances between bodies  
            if (r < (body[n].m + body[l].m)*(body[n].m + body[l].m)){

                /*update motion of the bodies */
                body[n].vx_temp = body[n].vx - 2*body[l].m/(body[n].m + body[l].m) * ( (body[n].vx-body[l].vx)*(body[n].x-body[l].x) + (body[n].vy-body[l].vy)*(body[n].y-body[l].y) )*(body[n].x - body[l].x)/r ;
                body[n].vy_temp = body[n].vy - 2*body[l].m/(body[n].m + body[l].m) * ( (body[n].vx-body[l].vx)*(body[n].x-body[l].x) + (body[n].vy-body[l].vy)*(body[n].y-body[l].y) )*(body[n].y - body[l].y)/r ;
                body[l].vx_temp = body[l].vx - 2*body[n].m/(body[n].m + body[l].m) * ( (body[n].vx-body[l].vx)*(body[n].x-body[l].x) + (body[n].vy-body[l].vy)*(body[n].y-body[l].y) )*(body[l].x-body[n].x)/r ;
                body[l].vy_temp = body[l].vy -2*body[n].m/(body[n].m + body[l].m) * ( (body[n].vx-body[l].vx)*(body[n].x-body[l].x) + (body[n].vy-body[l].vy)*(body[n].y-body[l].y) )*(body[l].y-body[n].y)/r ;
                
                body[n].vx = body[n].vx_temp; 
                body[n].vy = body[n].vy_temp;	
                body[l].vx = body[l].vx_temp; 
                body[l].vy = body[l].vy_temp;
                    
                //~ /* Balls bouncing on the floor: */
                //~ body[n].vx = 0.85*body[n].vx_temp; 
                //~ body[n].vy = 0.85*body[n].vy_temp;	
                //~ body[l].vx = 0.85*body[l].vx_temp; 
                //~ body[l].vy = 0.85*body[l].vy_temp;

                /*If one ball is inside other, get it out! */
                if( (body[n].x > body[l].x) && (body[n].y > body[l].y) ){  
                    body[n].x += 2*((float)rand())/RAND_MAX;
                    body[n].y += 2*((float)rand())/RAND_MAX;
                }else if ( (body[n].x < body[l].x) && (body[n].y > body[l].y) ){
                    body[n].x -= 2*((float)rand())/RAND_MAX;
                    body[n].y += 2*((float)rand())/RAND_MAX;
                }else if ( (body[n].x > body[l].x) && (body[n].y < body[l].y) ){
                    body[n].x += 2*((float)rand())/RAND_MAX;
                    body[n].y -= 2*((float)rand())/RAND_MAX;
                }else{
                    body[n].x -= 2*((float)rand())/RAND_MAX;
                    body[n].y -= 2*((float)rand())/RAND_MAX;
                }
                    
                    
                if (global_settings.INTERACTION == TRUE){
                /*update the status of the bodies involved in the collision */
                    
                /*if one of the bodies is infected, they both become infected */
                if (body[n].status == DOCTOR && body[l].status == INFECTED){ 
                    body[n].status = DOCTOR;
                    body[l].status = HEALTHY;
                }else if (body[n].status == INFECTED  && body[l].status == DOCTOR){ 
                    body[n].status = HEALTHY;
                    body[l].status = DOCTOR;
                }else if (body[n].status == DOCTOR && body[l].status == HEALTHY){ 
                    body[n].status = DOCTOR;
                    body[l].status = HEALTHY;
                }else if (body[n].status == HEALTHY  && body[l].status == DOCTOR){ 
                    body[n].status = HEALTHY;
                    body[l].status = DOCTOR;
                }else if (body[n].status == INFECTED && body[l].status == INFECTED){ 
                    body[n].status = INFECTED;
                    body[l].status = INFECTED;
                }else if (body[n].status == INFECTED && body[l].status == HEALTHY){ 
                    body[n].status = INFECTED;
                    body[l].status = INFECTED;
                    body[l].time_infection = N_iteracao * h;
                }else if (body[n].status == HEALTHY && body[l].status == INFECTED){ 
                    body[n].status = INFECTED;
                    body[n].time_infection = N_iteracao * h;
                    body[l].status = INFECTED;
                }else if (body[n].status == HEALTHY && body[l].status == HEALTHY){ 
                    body[n].status = HEALTHY;
                    body[l].status = HEALTHY;
                }else{}
                }
            }
        }
    }
        
     //////////   Check for collisions with exterior boundaries   //////////////  
     
    int Lx = global_settings.IMAGE_WIDTH - 12;
    int Ly = global_settings.IMAGE_HEIGHT - 12;
        
            
    if  (body[n].x - body[n].m < 12){    ////Verical left boundary
        body[n].x = 12 + 0.0001 + body[n].m; //safe condition for the boundary
        body[n].vx = -body[n].vx; 
        body[n].vy = body[n].vy;}
    if  (body[n].x + body[n].m >= Lx){ 
        body[n].x = Lx - 0.0001 - body[n].m;
        body[n].vx = -body[n].vx; 
        body[n].vy = body[n].vy;}    
    if  (body[n].y - body[n].m < 12){ // horizontal top boundary
        body[n].y = 12 + 0.0001 + body[n].m;
        body[n].vx = body[n].vx; 
        body[n].vy = -body[n].vy;
	    }
    if (body[n].y + body[n].m>= Ly) { // horizontal boundary
        body[n].y = Ly - 0.0001 - body[n].m;
        body[n].vx = body[n].vx; 
        body[n].vy = -body[n].vy;
	    }
    if  ( ( (body[n].x + body[n].m >= Lx * 0.49) && (body[n].x - body[n].m <= Lx * 0.51) )  &&  (body[n].y - body[n].m < 299) )
        { // Separation boundary
        if (body[n].x  - body[n].m <= Lx * 0.49) { body[n].x = Lx * 0.49 - 2 - body[n].m; }
        if (body[n].x  + body[n].m >= Lx * 0.51) { body[n].x = Lx * 0.51 + 2 + body[n].m; }
            
        body[n].vx = -body[n].vx; 
        body[n].vy =  body[n].vy;
        }
       
        body[n].vx = body[n].vx+body[n].ax*h; 
        body[n].vy = body[n].vy+body[n].ay*h;			
    }
    
    
    if (global_settings.INTERACTION == TRUE){
    //////////////   Count how many healthy bodies there are   ////////////////
    number_healthy = 0;
    number_doctors = 0;
    number_infected = 0;
    for (n = 0; n < N_bodies; n++)
        {
        if (body[n].status == HEALTHY) number_healthy++;   
        if (body[n].status == DOCTOR) number_doctors++;
        if (body[n].status == INFECTED) number_infected++;
         
        /* Check if there is casualities due to infection */
        if ( (body[n].status == INFECTED) && (N_iteracao * h - body[n].time_infection) > 5.0 )
            {
            //printf("Burn the witch! \n");
            //printf("N_iteracao * h = %f \n", N_iteracao * h);
            //printf("body[n].time_infection = %f \n", body[n].time_infection);
            body[n].delete = TRUE;
            memory_delete();
            }
        }
    }
    //printf("Percentage of infected people %f \n", 100.0*number_infected/(N_bodies - number_doctors) );
}





///////////////    DRAWING FUNCTIONS    ////////////////////////////////////////////////////////////////////////////////////////



static void do_drawing(cairo_t *cr) 
{
    cairo_pattern_t *pat;
    
    srand(time(NULL));
    
    /* draw walls */
    
    int Lx = global_settings.IMAGE_WIDTH - 12;
    int Ly = global_settings.IMAGE_HEIGHT - 12;
        
    cairo_set_line_width(cr, 3); 
    cairo_set_source_rgb(cr, 0, 0, 0);   // Walls Color
    cairo_move_to(cr, 12, 12);
    cairo_line_to(cr, Lx, 12);
    cairo_line_to(cr, Lx, Ly);
    cairo_line_to(cr, 12, Ly);
    cairo_line_to(cr, 12, 12);
    cairo_move_to(cr, Lx * 0.5, 12);
    cairo_line_to(cr, Lx * 0.5, 300);
    cairo_stroke (cr);
    
   
    for (unsigned int n = 0; n < N_bodies; n++){   // draw bodies
        cairo_set_line_width(cr, 9);  
        cairo_set_source_rgb(cr, 0, 0, 0);
    
        cairo_translate(cr, 0, 0);
        cairo_arc(cr, body[n].x, body[n].y, body[n].m, 0, 2 * M_PI);
        cairo_stroke_preserve(cr);
        
        if (body[n].selected == FALSE){    /*Draw an organism that is not selected */
             
             switch (body[n].status)
             {
                 case HEALTHY : {cairo_set_source_rgb(cr, 0, 0, 1);break;}
                 case DOCTOR : {cairo_set_source_rgb(cr, 0, 1, 0);break;}
                 case INFECTED : {cairo_set_source_rgb(cr, 1, 0, 0);break;}    
             }        
        }else{
        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_arc(cr, body[n].x, body[n].y, body[n].m, 0, 2 * M_PI);
        cairo_stroke_preserve(cr);
        pat = cairo_pattern_create_radial (body[n].x, body[n].y, 0, body[n].x, body[n].y, body[n].m);
        cairo_pattern_add_color_stop_rgba (pat, 0, 1, 0, 0, 1);
        cairo_pattern_add_color_stop_rgba (pat, 1, 1, 0.5, 0, 1);
        cairo_set_source (cr, pat);
        //cairo_set_source_rgb(cr, 255.0, 0, 0); 
        }
        cairo_fill(cr);
    }
    
    if (drawArrow  == TRUE)
    {
		double angle = atan2 (body[N_bodies - 1].ytemp - body[N_bodies - 1].y, body[N_bodies - 1].xtemp - body[N_bodies - 1].x) + M_PI;
	    float x1,x2,y1,y2;
		cairo_set_line_width(cr, 3); 
		cairo_set_source_rgb(cr, 1.0, 0.4, 0);   // Arrow Color
		cairo_move_to(cr, body[N_bodies - 1].x, body[N_bodies - 1].y);
        cairo_line_to(cr, body[N_bodies - 1].xtemp, body[N_bodies - 1].ytemp);
		
		////    Calculate arrow coordinates    ////
		
    	x1 = 15 * cos(angle - 0.5);
    	y1 = 15 * sin(angle - 0.5);
    	cairo_rel_line_to (cr, x1, y1);
    	cairo_rel_line_to (cr, -x1, -y1);
    	x2 = 15 * cos(angle + 0.5);
    	y2 = 15 * sin(angle + 0.5);
    	cairo_rel_line_to (cr, x2, y2);
    	
        cairo_stroke (cr);
	}
	
	if (draw_Select_Rectangle  == TRUE)
	{
		double dashes[] = {20.0,  /* ink */
                   5.0,  /* skip */
                   5.0,  /* ink */
                   5.0   /* skip*/
                  };
                  
        int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
        double offset = -50.0;

        //Prepare the pen to draw the rectangle
        cairo_set_dash (cr, dashes, ndash, offset);
        cairo_set_line_width (cr, 3.0);
        cairo_set_source_rgb(cr, 0, 0, 0);   // Rectangle color
        //Draw the rectangle!
        cairo_rectangle (cr, x_right_mouse_click, y_right_mouse_click, x_right_mouse_click_temp, y_right_mouse_click_temp);
        cairo_stroke (cr);
	}   
}

/* the following dawing routine is responsible for the mathematical chart */
static void do_drawing2(cairo_t *cr){
	
    cairo_set_source_surface(cr, plot_background, 10, 10);
    cairo_mask_surface(cr, plot_background, 10, 10);
    static gint count = 0;
    int Lx = global_settings.IMAGE_WIDTH - 12;
    
    static int time_series[500];
    static int time_series2[500];
    static int time_series3[500];
    
    int n;
    
    //////////////   Count how many healthy bodies there are   ////////////////
    number_healthy_B = 0;
    number_doctors_B = 0;
    number_infected_B = 0;
    
    for (n = 0; n < N_bodies; n++){
         if (body[n].status == HEALTHY  && (body[n].x + body[n].m >= Lx * 0.5) ) number_healthy_B++;   
         if (body[n].status == DOCTOR  && (body[n].x + body[n].m >= Lx * 0.5)) number_doctors_B++;
         if (body[n].status == INFECTED  && (body[n].x + body[n].m >= Lx * 0.5)) number_infected_B++;
    }

    if (N_bodies > 0){
    time_series[500] = 91 * number_healthy_B / N_bodies ;
    time_series2[500] = 91 * number_infected_B / N_bodies ;
    time_series3[500] = 91 * number_doctors_B / N_bodies ;
    }
    else{
    time_series[500] = 0;
    time_series2[500] = 0;
    time_series3[500] = 0;
    }
    
    for (n = 0; n < 500; n++){
       time_series[n]=time_series[n + 1]; 
       time_series2[n]=time_series2[n + 1]; 
       time_series3[n]=time_series3[n + 1]; 
    }
    
    cairo_set_source_rgb(cr, 0, 255, 0);
    
    for (n = 72; n < 490; n++){
        cairo_move_to(cr, n - 1, 112 - time_series3[n-1] );
        cairo_line_to(cr, n, 112 - time_series3[n] );
    }

    cairo_stroke(cr);
    
    cairo_set_source_rgb(cr, 255, 0, 0);

    for (n = 72; n < 490; n++){
        cairo_move_to(cr, n - 1, 112 - time_series2[n-1] );
        cairo_line_to(cr, n, 112 - time_series2[n] );
    }
  
    cairo_stroke(cr);
  
    cairo_set_source_rgb(cr, 0, 0, 255);
    
    for (n = 72; n < 490; n++){
        cairo_move_to(cr, n - 1, 112 - time_series[n-1] );
        cairo_line_to(cr, n, 112 - time_series[n] );
    }

    cairo_stroke(cr);
    count++;   
}





//////   ANIMATION FUNCTIONS    ////////////////////////////////////////



static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    
    /*If 'Space key is not pressed, then update the motion */   
    if  (SPACE_KEY_PRESSED == FALSE){  
        /* if the number of healthy bodies is 0, stop the simulation */
        if ( (number_healthy == 0 && N_bodies > 0) ){
            SIMULATION_END_DEATH = TRUE;
            printf("NO ONE IS LEFT ALIVE \n");   
        }
        //~ else if ( (number_infected == 0 && N_bodies > 0) ){
            //~ SIMULATION_END_LIFE = TRUE;
             //~ printf("mamas \n");   
        else{
            main_loop();
        }
    }
    if (widget == drawing_area){
        do_drawing(cr);
    return FALSE;
    }
    if (widget == drawing_area2){
        do_drawing2(cr);
    return FALSE;
}

}



static gboolean time_handler(GtkWidget *widget) {
    if (global_settings.SIMULATION_PAUSE == TRUE) return FALSE;
    gtk_widget_queue_draw(widget);

    return TRUE;
}







////////    MOUSE AND KEYBOARD FUNCTIONS    /////////////////////////////////////////




static gboolean clicked(GtkWidget *widget, GdkEventButton *event, gpointer user_data){
	if (event->button == 1) {
		
        N_bodies++;
        

	    
	    /* allocates arrays that are the size of N_bodies */
        body = (Body*)realloc(body, N_bodies * sizeof(Body));
        printf("Number of bodies = %i \n", N_bodies);
	    
	    body[N_bodies-1].body_id = N_bodies;
        body[N_bodies-1].selected = FALSE;
        body[N_bodies-1].delete = FALSE;
        body[N_bodies-1].status = HEALTHY;
        body[N_bodies-1].time_infection = N_iteracao * h;
        
        /*  Count how many healthy bodies there are   */
        number_healthy = 0;
        number_doctors = 0;
        number_infected = 0;
        for (int n = 0; n < N_bodies; n++){
            if (body[n].status == HEALTHY) number_healthy++;   
            if (body[n].status == DOCTOR) number_doctors++;
            if (body[n].status == INFECTED) number_infected++;        
        }
        
        if (D_KEY_PRESSED) body[N_bodies-1].status = DOCTOR;
        else if (A_KEY_PRESSED) body[N_bodies-1].status = INFECTED;
        else body[N_bodies-1].status = HEALTHY;
	    
        body[N_bodies-1].vx = 0;
	    body[N_bodies-1].vy = 0;
	    body[N_bodies-1].ax = 0;
	    body[N_bodies-1].ay = 0;
	    body[N_bodies-1].m = SIZE_TEMP;	
	    body[N_bodies-1].color.red = (float)(rand() % 2)*10 ; //Set random body colours
	    body[N_bodies-1].color.green = (float)(rand() % 2)*10 ;  
	    body[N_bodies-1].color.blue = (float)(rand() % 2)*10 ;  
	    
		body[N_bodies - 1].x = event->x + 0.00001*((float)rand())/RAND_MAX;
		body[N_bodies - 1].y = event->y + 0.00001*((float)rand())/RAND_MAX;
		body[N_bodies - 1].xtemp = event->x;
		body[N_bodies - 1].ytemp = event->y;
        LEFT_MOUSE_PRESSED = TRUE;
    }
    if (event->button == 2) {
    //
    }
    if (event->button == 3) {
        x_right_mouse_click = event->x;
        y_right_mouse_click = event->y;
        x_right_mouse_click_temp = x_right_mouse_click;
        y_right_mouse_click_temp = y_right_mouse_click;
        RIGHT_MOUSE_PRESSED = TRUE;
    }
        
    gtk_widget_queue_draw(widget);
    
    return TRUE;
}


static gboolean released(GtkWidget *widget, GdkEventButton *event, gpointer user_data){
	
    if (event->button == 1) {
        body[N_bodies - 1].vx = 5 * (body[N_bodies - 1].xtemp - body[N_bodies - 1].x);
        body[N_bodies - 1].vy = 5 * (body[N_bodies - 1].ytemp - body[N_bodies - 1].y);
        LEFT_MOUSE_PRESSED = FALSE;
        drawArrow = FALSE;    //it will stop drawing the arrow 
    }

    if (event->button == 3) {
        RIGHT_MOUSE_PRESSED = FALSE;
        draw_Select_Rectangle = FALSE;   // stop drawing the rectangle
        memory_delete();
    }   
    
    return TRUE;
}


static gboolean motion_notify_event( GtkWidget *widget, GdkEventMotion *event ){
	if (LEFT_MOUSE_PRESSED == TRUE){
	    drawArrow = TRUE;    //it will draw the arrow while left mouse button is pressed down
        body[N_bodies - 1].xtemp = event->x;
        body[N_bodies - 1].ytemp = event->y;
    }
    if (RIGHT_MOUSE_PRESSED == TRUE){
        x_right_mouse_click_temp = event->x - x_right_mouse_click;
        y_right_mouse_click_temp = event->y - y_right_mouse_click;
        draw_Select_Rectangle = TRUE;  //This draws the selection rectangle
        
        for (int n = 0; n < N_bodies; n++) {
            if ( (body[n].x > x_right_mouse_click && body[n].x < event->x) &&  (body[n].y > y_right_mouse_click && body[n].y < event->y) ){
            body[n].selected = TRUE;
            body[n].delete = TRUE;
            }
            else if ( (body[n].x < x_right_mouse_click && body[n].x > event->x) &&  (body[n].y > y_right_mouse_click && body[n].y < event->y) ){
            body[n].selected = TRUE;
            body[n].delete = TRUE;
            }
            else if ( (body[n].x > x_right_mouse_click && body[n].x < event->x) &&  (body[n].y < y_right_mouse_click && body[n].y > event->y) ){
            body[n].selected = TRUE;
            body[n].delete = TRUE;
            }
            else if ( (body[n].x < x_right_mouse_click && body[n].x > event->x) &&  (body[n].y < y_right_mouse_click && body[n].y > event->y) ){
            body[n].selected = TRUE;
            body[n].delete = TRUE;
            }
            else{
            body[n].selected = FALSE;
            body[n].delete = FALSE;
            }
	    }
           
    }
    gtk_widget_queue_draw(widget);
	return TRUE;
}


static gboolean mouse_scroll (GtkWidget *widget, GdkEventScroll *event, gpointer user_data){
    if (event->direction == 0  &&  LEFT_MOUSE_PRESSED == TRUE) /* this will increase the mass while scrolling up */
    {
        body[N_bodies - 1].m++;
        SIZE_TEMP = body[N_bodies - 1].m;
    }
    
    if (event->direction == 1  &&  LEFT_MOUSE_PRESSED == TRUE) /* this will decrease the mass while scrolling down */
    {
        body[N_bodies - 1].m--;
        SIZE_TEMP = body[N_bodies - 1].m;
    }
    return TRUE;  
}


gboolean keyboard_press (GtkWidget *widget, GdkEventKey *event, gpointer data){

/*Pause the simulation if the space key is pressed */
    if (event->keyval == GDK_KEY_space){
        if (SPACE_KEY_PRESSED == TRUE){ 
            SPACE_KEY_PRESSED = FALSE;}
        else{
            SPACE_KEY_PRESSED = TRUE;
        }    
    }
        
    if (event->keyval == GDK_KEY_a){
        if (A_KEY_PRESSED == TRUE){ 
            A_KEY_PRESSED = FALSE;}
        else{
            A_KEY_PRESSED = TRUE;
        }    
    }
    
        if (event->keyval == GDK_KEY_d){
        if (D_KEY_PRESSED == TRUE){ 
            D_KEY_PRESSED = FALSE;}
        else{
            D_KEY_PRESSED = TRUE;
        }    
    }
    return TRUE;     
};




///////////////////////////////   MEMORY MANAGEMENT    /////////////////



void memory_new(){    
    /*reset N_bodies and re-allocate memory for a 0 size array */
    N_bodies=0;
    body = (Body*)realloc(body, N_bodies * sizeof(Body));
    
    /*Reset number of healthy people, doctors and infected patients */
    number_healthy = 0; 
    number_doctors = 0;
    number_infected = 0; // ;

    /* Reset simulation state tags */
    SIMULATION_END_DEATH = FALSE;
    SIMULATION_END_LIFE = FALSE;
    
    /* reset keyboard pressed keys tags */
    A_KEY_PRESSED = FALSE;
    D_KEY_PRESSED = FALSE;
    SPACE_KEY_PRESSED = FALSE;
       
    /*reset the size of the bodies*/
    SIZE_TEMP = 10.0;
}


void memory_delete(){
    
    //Loop variables are declared in the register to make code faster
    register int n;
    
    Body *body_copy;
    
    //Count how many bodies are there for deletion:
    int dummy = 0;

    for (n=0; n<N_bodies; n++){
        if (body[n].delete == FALSE){
	        dummy++;
        }
    }
	
    body_copy = (Body*)malloc(dummy * sizeof(Body));

    
    dummy=0;
    
    for (n=0; n<N_bodies; n++){
        if (body[n].delete == FALSE){
	        body_copy[dummy]=body[n];
            dummy++;
        }
    }
    		
    body= (Body*)realloc(body,dummy * sizeof(Body));

    for (n=0; n<dummy; n++){
        body[n]=body_copy[n];
    }

    N_bodies=dummy;
    free(body_copy);

} 
    
    
    
    
    
static void button_clicked(GtkWidget *button, gpointer data){

 for (int n = 0; n < N_bodies; n++){
body[n].vx = - body[n].vx;
body[n].vy = - body[n].vy;
 }

}
    

  


///////////////////////////////   MAIN! ////////////////////////////////

int main(int argc, char *argv[])
{
    GtkWidget *window;
    GtkBuilder *builder;
    GObject *button;
    GObject *GTKMenuItem;
    GObject *GTKMenuItem_file_new;
    
    init_vars();    // call function to initiate the variables

    gtk_init(&argc, &argv);
   
    /* Construct a GtkBuilder instance and load our UI description */
    builder = gtk_builder_new ();
    gtk_builder_add_from_file (builder, "CSD.ui", NULL);
    
     /* Connect signal handlers to the constructed widgets. */
    window = gtk_builder_get_object (builder, "main_window");
    button = gtk_builder_get_object (builder, "reverse_button");
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    
    GTKMenuItem = gtk_builder_get_object (builder, "file_quit");
    GTKMenuItem_file_new = gtk_builder_get_object (builder, "file_new");
    g_signal_connect (GTKMenuItem, "activate", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (GTKMenuItem_file_new, "activate", G_CALLBACK (memory_new), NULL);
    
    drawing_area = gtk_builder_get_object(builder, "draw_area");
    drawing_area2 =  gtk_builder_get_object(builder, "draw_area2");
  
    //chart = slope_chart_new();
    
    gtk_widget_set_size_request(drawing_area, global_settings.IMAGE_WIDTH, global_settings.IMAGE_HEIGHT);
    gtk_widget_set_size_request(drawing_area2, global_settings.IMAGE_WIDTH, 150);
 
    /* Enable the drawing area to recognize mouse and keyboard events */
    gtk_widget_add_events(drawing_area, GDK_BUTTON_PRESS_MASK);
    gtk_widget_add_events(drawing_area, GDK_BUTTON_RELEASE_MASK);
    gtk_widget_add_events(drawing_area, GDK_SCROLL_MASK);
    gtk_widget_add_events(drawing_area, GDK_BUTTON_MOTION_MASK);
    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);
      
    /* Signals */ 
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    g_signal_connect(G_OBJECT(drawing_area2), "draw", G_CALLBACK(on_draw_event), NULL);
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(on_draw_event), NULL);
    g_signal_connect(drawing_area, "button-press-event", G_CALLBACK(clicked), NULL);
    g_signal_connect(drawing_area, "button-release-event", G_CALLBACK(released), NULL);
    g_signal_connect (drawing_area, "motion_notify_event", G_CALLBACK(motion_notify_event), NULL);
    g_signal_connect(drawing_area, "scroll-event", G_CALLBACK(mouse_scroll), NULL);
    g_signal_connect (G_OBJECT(window), "key_press_event", G_CALLBACK (keyboard_press), NULL);    
    g_signal_connect(GTK_BUTTON(button), "clicked", G_CALLBACK(button_clicked), window);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), global_settings.IMAGE_WIDTH, global_settings.IMAGE_HEIGHT); 
    gtk_window_set_title(GTK_WINDOW(window), "Cairo System Dynamics 0.15");


    g_timeout_add(20, (GSourceFunc) time_handler, (gpointer) window);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;    
}
