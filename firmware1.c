/* x20-radio/remote.c */
/* Copyright (c) 2020 J. M. Spivey */

#include "microbian.h"
#include "hardware.h"
#include "lib.h"

#define GROUP 17

#define SPEAKER_PIN DEVPIN(0, 0)

static int EMOJI_COUNT = 3;
int compteur = 0;

static const image emoji_A =
    IMAGE(0,1,0,1,0,
          0,1,0,1,0,
          0,1,0,1,0,
          1,0,0,0,1,
          0,1,1,1,0);

static const image emoji_B =
    IMAGE(0,1,0,1,0,
          0,1,0,1,0,
          0,1,0,1,0,
          0,1,1,1,0,
          1,0,0,0,1);

static const image emoji_C =
    IMAGE(0,0,0,0,0,
          1,1,0,1,1,
          0,0,0,0,0,
          1,1,1,1,1,
          0,0,0,0,0);

void emoji_received(int dummy)
{
    byte buf[RADIO_PACKET];

    int n;
    while (1) {
        n = radio_receive(buf);
        if(n == 1){
            // BEEP !
            gpio_out(SPEAKER_PIN, 1); // Allumer
            timer_delay(100); // 0.1 sec de beep
            gpio_out(SPEAKER_PIN, 0); // Eteindre
            printf("Recived %c\n", buf[0]);
            compteur = buf[0] - '0';
        }
        timer_delay(100);
    }
}

void emoji_choice(int dummy){

    gpio_connect(BUTTON_A);
    gpio_connect(BUTTON_B);
    // int compteur = 0;

    // 0 = DOWN
    // 1 = UP
    int a_lecture = 1;
    int b_lecture = 1;

    int last_state_a = 1;
    int last_state_b = 1;

    int on_a_up = 0;
    int on_b_up = 0;

    while(1){
        // Gestion de l'input
        // Le choix à été fait de ne changer qu'au relachement du bouton, moins réactif mais plus juste pour l'envois (évite de se tromper)
        a_lecture = gpio_in(BUTTON_A);
        b_lecture = gpio_in(BUTTON_B);

        if(last_state_a != a_lecture && a_lecture == 1){
            on_a_up = 1;
        }

        if(last_state_b != b_lecture && b_lecture == 1){
            on_b_up = 1;
        }

        last_state_a = a_lecture;
        last_state_b = b_lecture;

        // Réponse à l'input
        if (on_a_up == 1 && on_b_up == 1){
            // on send à l'autre le compteur actuel
            char sending[1];
            sending[0] = compteur + '0';
            radio_send(sending, 1);
            on_a_up = 0;
            on_b_up = 0;
        }
        else if(on_a_up == 1){
            compteur--;
            on_a_up = 0;
        }
        else if(on_b_up == 1){
            compteur++;
            on_b_up = 0;
        }

        // Gérer comme ça, le modulo gérant pas les négatifs correctement
        if(compteur < 0){
            compteur = EMOJI_COUNT - 1;
        }
        if(compteur >= EMOJI_COUNT){
            compteur = 0;
        }
        timer_delay(100);
    }
}

// Tâche responsable de l'affichage
void main_task(int dummy){
    while(1){
        switch(compteur){
            case(0):
                display_show(emoji_A);
                break;
            case(1):
                display_show(emoji_B);
                break;
            case(2):
                display_show(emoji_C);
                break;
        }
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

    start("Receiver", emoji_received, 0, STACK);
    start("Choice", emoji_choice, 1, STACK);
    start("Main", main_task, 2, STACK);
}