/* Copyright (c) 2020 J. M. Spivey */

#include "microbian.h"
#include "hardware.h"
#include "lib.h"

#define GROUP 17

#define SPEAKER_PIN DEVPIN(0, 0)

// Représentation en int, ce avec quoi on travaille
unsigned int drawing[5][5] = 
{
    {0,0,0,0,0},
    {0,0,1,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0}
};

unsigned int displayed_image[5][5]= {
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0}
};
image final_image = IMAGE(
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0
);

int displaying_recieved = 0;

// Méthodes pour manipuler le dessin
void send_drawing(){
    char sending[25];
    for(int x = 0; x < 5; x++){
        for(int y = 0; y < 5; y++){
            sending[x*5 + y] = drawing[x][y] + '0';
        }
    }
    radio_send(sending, 25);
}

void switch_pixel(int x, int y){
    if(drawing[x][y] == 0){
        drawing[x][y] = 1;
    }
    else{
        drawing[x][y] = 0;
    }
}

int getRowX(int x){
    switch(x){
        case 0:
            return ROW1;
        case 1:
            return ROW2;
        case 2:
            return ROW3;
        case 3:
            return ROW4;
        case 4:
            return ROW5;
    }
}
void apply_to_image(unsigned int drawing[5][5]){
    for(int i = 0; i < 5; i++){
        final_image[i*2] = BIT(getRowX(i)) | (!drawing[i][0]<<28) | (!drawing[i][1]<<11) | (!drawing[i][2]<<31) | (!drawing[i][4]<<30);
        final_image[i*2 + 1] =  (!drawing[i][3]<<5);
    }
}
void copy_image(unsigned int mat[5][5]){
    for(int x = 0; x < 5; x++){
        for(int y = 0; y < 5; y++){
            displayed_image[x][y] = mat[x][y];
        }
    }
}

void drawing_received(int dummy)
{
    byte buf[RADIO_PACKET];
    int n;
    while (1) {
        n = radio_receive(buf);
        if(n == 25){
            // BEEP !
            gpio_out(SPEAKER_PIN, 1); // Allumer
            timer_delay(100); // 0.1 sec de beep
            gpio_out(SPEAKER_PIN, 0); // Eteindre

            printf("Recived %c\n", buf[0]);
            unsigned int recv_drawing[5][5] ={
                {0,0,0,0,0},
                {0,0,0,0,0},
                {0,0,0,0,0},
                {0,0,0,0,0},
                {0,0,0,0,0}
            };
            for(int x = 0; x < 5; x++){
                for(int y = 0; y < 5; y++){
                    recv_drawing[x][y] = buf[x * 5 + y] -'0';
                }
            }
            copy_image(recv_drawing);
            displaying_recieved = 1;
        }
        timer_delay(100);
    }
}

int cursor_x = 0;
int cursor_y = 0;

void drawing_choice(int dummy){

    gpio_connect(BUTTON_A);
    gpio_connect(BUTTON_B);

    int longpress_time = 500;

    // 0 = DOWN
    // 1 = UP
    int a_state = 1;
    int b_state = 1;

    int last_state_a = 1;
    int last_state_b = 1;

    int on_a_up = 0;
    int on_b_up = 0;

    int time_press_a = 0;
    int time_press_b = 0;

    int start_time_a = 0;
    int start_time_b = 0;

    while(1){
        // Gestion de l'input
        // Le choix à été fait de ne changer qu'au relachement du bouton, moins réactif mais plus juste pour l'envois (évite de se tromper)
        a_state = gpio_in(BUTTON_A);
        b_state = gpio_in(BUTTON_B);

        // on up
        if(last_state_a != a_state && a_state == 1){
            on_a_up = 1;
        }
        if(last_state_b != b_state && b_state == 1){
            on_b_up = 1;
        }
        // on down
        if(last_state_a != a_state && a_state == 0){
            start_time_a = timer_now();
            time_press_a = 0;
            displaying_recieved = 0;
        }
        if(last_state_b != b_state && b_state == 0){
            start_time_b = timer_now();
            time_press_b = 0;
            displaying_recieved = 0;
        }


        // Réponse à l'input
        // si relache les deux boutons après les avoir maintenus.
        if (time_press_a > longpress_time && time_press_b > longpress_time && on_a_up == 1 && on_b_up == 1){
            on_a_up = 0;
            on_b_up = 0;
            // on send à l'autre l'emoji créé
            send_drawing();
        }
        // Si on en relache un seul après maintiens
        else if(time_press_a > longpress_time && b_state == 1 && on_a_up == 1)
        {
            on_a_up = 0;
            switch_pixel(cursor_x, cursor_y);
            copy_image(drawing);
        }
        else if(time_press_b > longpress_time && a_state == 1 && on_b_up == 1){
            on_b_up = 0;
            switch_pixel(cursor_x, cursor_y);
            copy_image(drawing);
        }
        // Si on a pas maintenu, et qu'on relache
        else if(on_a_up == 1){
            cursor_y++;
            on_a_up = 0;
            copy_image(drawing);
        }
        else if(on_b_up == 1){
            cursor_x++;
            on_b_up = 0;
            copy_image(drawing);
        }

        // Gérer comme ça, le modulo gérant pas les négatifs correctement
        if(cursor_x >= 5){
            cursor_x = 0;
        }
        if(cursor_y >= 5){
            cursor_y = 0;
        }

        // Update des timers, en fin de boucle pour captuer la loop avec le up après long press
        if(a_state == 0){
            time_press_a =  timer_now() - start_time_a;
        }
        else{
            time_press_a = 0;
        }
        if(b_state == 0){
            time_press_b =  timer_now() - start_time_b;
        }
        else{
            time_press_b = 0;
        }

        last_state_a = a_state;
        last_state_b = b_state;
        timer_delay(100);
    }
}
// Tâche responsable de l'affichage
void main_task(int dummy){
    while(1){
        // displayed_image[4][4] = displaying_recieved;
        apply_to_image(displayed_image);
        display_show(final_image);
    }
} 

// Task à part, pour pouvoir avoir un delay sans faire "lagger" le display
void blink_task(int dummy){
    while(1){
        if(displaying_recieved == 0){
            // Le blink du curseur si on est pas en train de montrer un dessin reçu
            displayed_image[cursor_x][cursor_y] = 0;
            timer_delay(200);
            displayed_image[cursor_x][cursor_y] = 1;
            timer_delay(200);
        }
        timer_delay(100);
    }
}

void init(void)
{
    serial_init();
    radio_init();
    radio_group(GROUP);
    gpio_dir(SPEAKER_PIN, 1); // Output
    gpio_drive(SPEAKER_PIN, 1); // 1V
    timer_init();
    display_init();

    start("Receiver", drawing_received, 2, STACK);
    start("Blink", blink_task, 1, STACK);
    start("Drawing", drawing_choice, 3, STACK);
    start("Main", main_task, 2, STACK);
}