#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"
#include "assign02.pio.h"
#include "hardware/pwm.h"
#include <time.h>

#define IS_RGBW true        
#define NUM_PIXELS 1        
#define WS2812_PIN 28       
#define MAX_STRING_SIZE 20
#define ARRAY_SIZE 173

//Flags for the game
int disp_morse = 1;
int letter = 1;

//store the win streak
int wincount=0; 

//Lives
int lives = 3;

//used for calculatesStats function
int rightinpt;
int wronginpt;

//used for add input function 
char currntinpt[200];
int currentind = -1;

//for alarm timer
absolute_time_t start_time;
typedef struct Morse Morse;
struct Morse{
    char morse_name[MAX_STRING_SIZE];
    char letter;
};

Morse* createMorse(char* name, char letter){
    Morse* new_morse = (Morse*) malloc(sizeof(Morse));
    strcpy(new_morse->morse_name, name);
    new_morse->letter = letter;
    return new_morse;
}


Morse* rand_array[36];

char rand_letters[36] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";


char letters[36] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";


char morse_data[36][6] = {
    {".-"},
    {"-..."},
    {"-.-."},
    {"-.."},
    {"."},
    {"..-."},
    {"--."},
    {"...."},
    {".."},
    {".---"},
    {"-.-"},
    {".-.."},
    {"--"},
    {"-."},
    {"---"},
    {".--."},
    {"--.-"},
    {".-."},
    {"..."},
    {"-"},
    {"..-"},
    {"...-"},
    {".--"},
    {"-..-"},
    {"-.--"},
    {"--.."},
    {".----"},
    {"..---"},
    {"...--"},
    {"....-"},
    {"....."},
    {"-...."},
    {"--..."},
    {"---.."},
    {"----."},
    {"-----"},
};




Morse* hashTable[ARRAY_SIZE];


int hashfunc(char* s){
    int hash = 0;
    while(*s){
        hash = 1 + (hash + *s) % (ARRAY_SIZE-1);
        s++;
    }
    return hash;
}


int intsearch (char* name){
    int hash_num = hashfunc(name);
    int start = hash_num;
    while(hashTable[hash_num] != NULL){
        if(strcmp(hashTable[hash_num]->morse_name, name)==0){
            return hash_num;
        }
    hash_num = hash_num + 1;
        if(hash_num == ARRAY_SIZE){
            hash_num = 0;
        }
        if(hash_num == start){
            return -1;
        }
    }
    return hash_num;
}


void insert(){
    for(int i = 0; i < 36; i++){
        int hash_num = intsearch(morse_data[i]);
        if(hash_num == -1){
            printf("The table is full on loading!");
        }
        else{
            Morse* new_morse = createMorse(morse_data[i], letters[i]);
            hashTable[hash_num] = new_morse;
        }
    }
}
void start_timer(){
    start_time = get_absolute_time();
}


int end_timer(){
    int end = (int) absolute_time_diff_us(start_time, get_absolute_time());
    return end/200000;
}


void add_input(int sample, int correct_index){
    currentind = currentind - correct_index;
    if (currentind < 200){
        if (sample == 0){
            currntinpt[currentind] = '.';
        }
        else if (sample == 1){
            currntinpt[currentind] = '-';
        }
        else if (sample == 2){
            currntinpt[currentind] = ' ';
        }
        else if(sample == 3){
            currntinpt[currentind] = '\0';
        }
    }
    currentind++;
    if (correct_index){
        printf("\b%c",currntinpt[currentind-1]);
    }
    else printf("%c",currntinpt[currentind-1]);

}

void calculateStats(int reset){
    printf("\n\n\t\t****************Scoreboard****************");
    printf("\n\t\t*Attemps: \t\t\t\t%d*", rightinpt+wronginpt);
    printf("\n\t\t*Correct: \t\t\t\t%d*", rightinpt);
    printf("\n\t\t*Incorrect: \t\t\t\t%d*", wronginpt);
    printf("\n\t\t*Win Streak: \t\t\t\t%d*", wincount);
    printf("\n\t\t*Lives Left: \t\t\t\t%d*\n", lives);
    if (rightinpt!=0 || wronginpt!=0){
        float stat = rightinpt * 100 / (rightinpt + wronginpt);
        if (reset){
            rightinpt = 0;
            wronginpt = 0;
            printf("\t\t*Correct %% for this level: \t%.2f%%*\n",stat); 
        }
        else {
            printf("\t\t*Correct Percent :\t\t\t%.2f%%*\n",stat);
        }
    }
    printf("\t\t****************Scoreboard****************\n\n");
}
int level(int l){
    if (l == 1){
        disp_morse = 1;
        letter = 1;
    }
    else if (l == 2){
        disp_morse = 0;
        letter = 1;
    }
    else if (l == 3){
        disp_morse = 1;
        letter = 0;
    }
    else if (l == 4){
        disp_morse = 0;
        letter = 0;
    }
    return l;
}

void start_the_game(){
    set_blue_on();
    printf("\n\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("| *               * * * * * * *          *          * * * * * *    *           * |\n");
    printf("| *               *                    *   *        *           *  * *         * |\n");
    printf("| *               *                  *       *      *           *  *   *       * |\n");
    printf("| *               * * * * *        *           *    * * * * * *    *     *     * |\n");
    printf("| *               *               * * * * * * * *   *      *       *       *   * |\n");
    printf("| *               *               *             *   *        *     *         * * |\n");
    printf("| * * * * * * *   * * * * * * *   *             *   *          *   *           * |\n");
    printf("|                                                                                |\n");
    printf("|                                                                                |\n");


    printf("| *           *   ");    printf("* * * * * * *  ");     printf("  * * * * * *  ");   printf("     * * * * *     ");       printf("* * * * * * * | \n"); 
    printf("| * *       * *   ");    printf("*           *  ");     printf("  *          * ");   printf("    *          *   ");       printf("*             |\n");
    printf("| *   *   *   *   ");    printf("*           *  ");     printf("  *          * ");   printf("    *              ");       printf("*             | \n");
    printf("| *     *     *   ");    printf("*           *  ");     printf("  * * * * * *  ");   printf("     * * * * *     ");       printf("* * * * *     | \n");
    printf("| *           *   ");    printf("*           *  ");     printf("  *       *    ");   printf("               *   ");       printf("*             | \n");
    printf("| *           *   ");    printf("*           *  ");     printf("  *        *   ");   printf("    *          *   ");       printf("*             |  \n");
    printf("| *           *   ");    printf("* * * * * * *  ");     printf("  *         *  ");   printf("     * * * * *     ");       printf("* * * * * * * | \n");

    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("\n\n\n\n\n\n\t\t***********************************************\n");
    printf("\t\t*                                             *\n");
    printf("\t\t* Enter .---- for level 1 (Characters - Easy) *\n");
    printf("\t\t* Enter ..--- for level 2 (Characters - Hard) *\n");
    printf("\t\t* Enter ...-- for level 3 (Words - Easy)      *\n");
    printf("\t\t* Enter ....- for level 4 (Words - Hard)      *\n");
    printf("\t\t*                                             *\n");
    printf("\t\t***********************************************\n");
    currentind = -1;
    main_asm();
    printf("\n\n\n\n\n\n");
    int current_level;
    // add a loop so that if the input is wrong there is no
    if (strcmp(currntinpt, ".----") == 0){
        current_level = level(1);
    }
    else if (strcmp(currntinpt, "..---") == 0){
        current_level = level(2);
    }
    else if (strcmp(currntinpt, "...--") == 0){
        current_level = level(3);
    }
    else if (strcmp(currntinpt, "....-") == 0){
        current_level = level(4);
    }
    else {
        printf("\t\tWrong Input\n\t\tExiting\n");
        return;
    }
    set_correct_led();
    int num_count = rand() % 36;
    char current_letter;
    char* current_morse;
    char current_word[4];
    char current_morse2[50] = "";
    char space[2] = " \0";

    while(lives > 0){
        printf("\n\n\n\n\n\n\n\n\n\n\n");
        if(letter == 1){
            current_letter = rand_array[num_count]->letter;
            current_morse = rand_array[num_count]->morse_name;
            if(disp_morse == 1){ 
                printf("\n\t\tYour Challenge is: %c \n\t\tand %c in Morse is %s:\n", current_letter, current_letter,current_morse);
            }
            else
                printf("\n\t\tYour Challenge is: %c\n", current_letter);
        }
        else {
            for (int i = 0; i < 3; i++){
                current_word[i] = rand_array[num_count]->letter;
                strcat(current_morse2, rand_array[num_count]->morse_name);
                if(i != 2){
                    strcat(current_morse2, space);
                }
                num_count = rand() % 36;
            }
            current_word[3] = '\0';
            current_morse = current_morse2;
            if(disp_morse == 1){
                printf("\n\t\tYour Challenge is: %s \n\t\tand %s in Morse, it's %s:\n", current_word, current_word, current_morse);
            }
            else
                printf("\n\t\tYour Challenge is: %s\n", current_word);
        }
        printf("\t\tEnter the Morse Code after the prompt\n");
        currentind = -1;
        main_asm();
        if (currentind == 0){
            printf("\t\tNo update detected\nGenerating New Level\n");
            continue;
        }
        int pos = intsearch(currntinpt);
        if(strcmp(current_morse,currntinpt)==0){
            rightinpt++;
            wincount++;
            printf("\t\tCorrect!\n");
            if (lives < 3)lives = lives + 1;
            if (wincount == 5){
                if (current_level == 4){
                    printf("\t\tCongrats on completimg the game\n");
                    winning_sequence();
                    break;
                }
                current_level = level(current_level+1);
                printf("\n\n\t\t*************************************************************\n\n");
                printf("\t\t\t\tYou are advancing to Level %d\n\n",current_level);
                printf("\t\t*************************************************************\n\n");
                calculateStats(1);
                wincount = 0;
            }
        }
        else{
            wronginpt++;
            printf("Incorrect :(\n");
            if (strcmp(hashTable[pos]->morse_name, "")==0){
                printf("\t\tEntered morse Code does not exist\n");
            }
            else if (letter){
                printf("\t\tMorse Code entered is for %c\n",hashTable[pos]->letter);
            }
            lives = lives - 1;
            wincount = 0;
        }
        num_count = rand() % 36;
        set_correct_led();
        currntinpt[0] = '\0';
        current_morse2[0] = '\0';
        if (rightinpt+wronginpt)calculateStats(0);
    }
    calculateStats(1);
    if(lives == 0)set_red_on();
    printf("\n\n\n\n\n\n\t\t*****************************\n");
    printf("\t\t*                           *\n");
    printf("\t\t* Enter ...-- to play again *\n");
    printf("\t\t* Enter .---- to exit       *\n");
    printf("\t\t*                           *\n");
    printf("\t\t*****************************\n");
    currentind = -1;
    main_asm();
    if (currentind == 0){
        printf("\t\tNo update detected\n\t\tProgram Will now Exit.");
    }
    if (strcmp(currntinpt, "...--") == 0){
        start_the_game();
    }
}
// Declare the main assembly entry point before use.
void main_asm();

// Button event detection handler and Morse code input buffer logic
// should be implemented in ARM assembly
void asm_gpio_init(uint pin) {
    gpio_init(pin);
}


void asm_gpio_set_dir(uint pin, bool out) {
    gpio_set_dir(pin, out);
}


bool asm_gpio_get(uint pin) {
    return gpio_get(pin);
}


void asm_gpio_put(uint pin, bool value) {
    gpio_put(pin, value);
}


void asm_gpio_set_irq(uint pin){
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}


static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}


static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return  ((uint32_t) (r) << 8)  | ((uint32_t) (g) << 16) | (uint32_t) (b);
}


void set_color_led_off(){
    put_pixel(urgb_u32(0x00, 0x00, 0x00));
}


void set_red_on(){
    put_pixel(urgb_u32(0x3F, 0x00, 0x00));
}


void set_green_on(){
    put_pixel(urgb_u32(0x00, 0x3F, 0x00));
}


void set_yellow_on(){
    put_pixel(urgb_u32(0x0F,0x0F,0x0F));
}

void set_orange_on(){
    put_pixel(urgb_u32(0x3F, 0xA5, 0x00));s
}


void set_blue_on(){
    put_pixel(urgb_u32(0x00, 0x00, 0x3F));
}


void winning_sequence(){
    while(true){
        set_blue_on();
        sleep_ms(500);
        set_green_on();
        sleep_ms(500);
        set_yellow_on();
        sleep_ms(500);
        set_orange_on();
        sleep_ms(500);
        set_red_on();
    }
}


void set_correct_led(){
    if (lives == 1)set_orange_on();
    else if (lives == 2)set_yellow_on();
    else set_green_on();
    return ;
}
void init_random_morse(){
    for(int i = 0; i < 36; i ++){
        rand_array[i] = createMorse(morse_data[i], letters[i]);
    }
}


void swap_array(int x, int y){ 
    Morse* temp = rand_array[x];
    rand_array[x] = rand_array[y];
    rand_array[y] = temp;
}


void finrandper36(){
    init_random_morse();
    srand(time(NULL));
    absolute_time_t time = get_absolute_time();
    int randgen = to_ms_since_boot(time);
    srand(randgen);
    for(int i = 0; i < 36; i++){
        int x = rand() % 36;
        swap_array(i, x);
    }
}

void dealloc(){
    for(int i = 0; i < ARRAY_SIZE; i++){
        free(hashTable[i]);
    }
    for(int i = 0; i < 36; i++){
        free(rand_array[i]);
    }
}

/*
 * Main entry point for the code - simply calls the main assembly function.
 */
int main() {
    stdio_init_all();
    // Initialise the PIO interface with the WS2812 code
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, 0, offset, WS2812_PIN, 800000, IS_RGBW);
    finrandper36(); 
    insert();
    watchdog_enable(0x7fffff, 1);
    start_the_game();
    dealloc();
    printf("\t\tGame has ended\n");
    return(0);
}

