#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/keyboard.h>
#include <linux/input.h>
#include <linux/notifier.h>
#include <linux/kthread.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Delmeus, okejka1, asiazo");
MODULE_DESCRIPTION("Konami code listener");

#define KEYBOARD_IRQ 1  // Numer przerwania klawiatury
#define KEY_UP 103      // Kod klawisza dla strzałki w gore
#define KEY_DOWN 108	// Kod klawisza dla strzałki w dol
#define KEY_RIGHT 106	// Kod klawisza dla strzałki w prawo
#define KEY_LEFT 105	// Kod klawisza dla strzałki w lewo
#define DELAY_MS 160   	// Opóźnienie w milisekundach

//urzadzenie to wciskania klawiszy po wcisnieciu konami
static struct input_dev *input_device;

//aby moc odpalic dodatkowy watek
static struct task_struct *konami_thread;

static unsigned long last_key_time = 0;

//tablica do sprawdzania stanu konami code
static int konami[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//oczekiwana sekwencja
static int expected_seq[8] = {KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT};

//licznik do sprawdzania sekwencji
static int counter = 0;

//ostatni wcisniety przycisk
static int lastCode;

//czy wprowadzono konami lub czy wcisnieto stop 
static bool konami_code_entered = false;

static void press_Enter(void);

//funkcja do wywolania zewnetrznego programu
static int display_easter_egg(void *data)
{
	//definujemy wprowadzane wymbole
	const char *first_line = "/\\_/\\";
	const char *second_line = "(o.o)";
    	const char *third_line = "> ^ <";
    	int i;
    	int keycode;
	while(!kthread_should_stop()){
		/*
		* jezeli wprowadzono konami wypisujemy kota w nieskonczonosc
		*/
		while(konami_code_entered){
			press_Enter();
			for (i = 0; i < strlen(first_line); i++) {
				char ch = first_line[i];

				if (ch == '/') {
				    keycode = KEY_SLASH;
				} else if (ch == '\\') {
				    keycode = KEY_BACKSLASH;
				} else if (ch == '_') {
				    keycode = KEY_MINUS;
				} else {
				    continue;
				}

				input_report_key(input_device, keycode, 1);  //wcisniecie przycisku
				input_sync(input_device);
				msleep(10);
				input_report_key(input_device, keycode, 0);  //zwolnienie przycisku
				input_sync(input_device);
			}
			press_Enter();
			for (i = 0; i < strlen(second_line); i++) {
				char ch = second_line[i];

				if (ch == '(') {
				    keycode = KEY_KPLEFTPAREN;
				} else if (ch == ' ') {
				    keycode = KEY_SPACE;
				} else if (ch == 'o') {
				    keycode = KEY_O;
				} else if (ch == '.') {
				    keycode = KEY_DOT;
				} else if (ch == ')') {
				    keycode = KEY_KPRIGHTPAREN;
				} else {				
				    continue;
				}

				input_report_key(input_device, keycode, 1); 
				input_sync(input_device);
				msleep(10);
				input_report_key(input_device, keycode, 0);
				input_sync(input_device);
			}
			press_Enter();
			for (i = 0; i < strlen(third_line); i++) {
				char ch = third_line[i];

				if (ch == '>') {
				    keycode = KEY_DOT;
				} else if (ch == ' ') {
				    keycode = KEY_SPACE;
				} else if (ch == '^') {
				    keycode = KEY_6;
				} else if (ch == '<') {
				    keycode = KEY_COMMA;
				} else {
				    continue;
				}
				if(keycode == KEY_DOT || keycode == KEY_COMMA || keycode == KEY_6){
					input_report_key(input_device, KEY_LEFTSHIFT, 1);
					input_sync(input_device);
					input_report_key(input_device, keycode, 1); 
					input_sync(input_device);
					msleep(10);
					input_report_key(input_device, KEY_LEFTSHIFT, 0);
					input_sync(input_device);
					input_report_key(input_device, keycode, 0);
					input_sync(input_device);
				}
				else{
					input_report_key(input_device, keycode, 1);  
					input_sync(input_device);
					msleep(10);
					input_report_key(input_device, keycode, 0);
					input_sync(input_device);
				}
			}
			msleep(100);
		}
		msleep(100);
		
    	}
    	return 0;
}

static void press_Enter(void){
	msleep(10);
	input_report_key(input_device, KEY_ENTER, 1); 
	input_sync(input_device);
	msleep(10);
	input_report_key(input_device, KEY_ENTER, 0);
	input_sync(input_device);
	msleep(10);
}

static int keyboard_interrupt_handler(struct notifier_block *nblock, unsigned long code, void *_param)
{

	struct keyboard_notifier_param *param = _param;
	//przypisujemy czas wcisniecia
	unsigned long current_time = jiffies_to_msecs(jiffies);

	if (code == KBD_KEYCODE ) {
		/*
		*	sprawdzenie czy minelo wystarczajaco duzo czasu od 
		*	ostatniego wcisniecia
		*/
		if (current_time - last_key_time >= DELAY_MS) {
			/*
			*	jezeli od ostatniego wcisniecia minelo
			*	wiecej niz 3 sekundy to resetujemy sekwencje
			*/
			if(current_time - last_key_time > 3000){
				for(int i = 0; i < 8; i++)
					konami[i] = 0;
				counter = 0;
			}
			/*
			*	jezeli wcisnieto jakakolwiek strzalke
			*/
			if(param->value == KEY_UP || param->value == KEY_DOWN || param->value == KEY_LEFT || param->value == KEY_RIGHT ){
				lastCode = param->value;      
		    		last_key_time = current_time;
		    		/*
		    		* jesli kilka razy wcisniemy strzalke w gore
		    		* to nadpisujemy poprzednie wcisniecia 
		    		* zamiast od razu resetowac sekwencje
		    		*/ 				  				  	
		  		if(konami[0] == KEY_UP && konami[1] == KEY_UP && counter == 2 && lastCode == KEY_UP){
		  			counter = 1;
		  		}
		  		
		  		konami[counter] = lastCode;
		  		printk("Konami[%d] = %d", counter, lastCode);
		  		
		  		//porownujemy aktualny kod do oczekiwanego
		  		if(konami[counter] != expected_seq[counter]){
		  			//reset sekwencji jesli nie zgadza sie z oczekiwana
		  			for(int i = 0; i < 8; i++)
						konami[i] = 0;
					//counter na -1 poniewaz zaraz zwiekszamy go o 1
					counter = -1;
					printk("Resetting sequence\n");
		  		}
		  		/*
		  		* jezeli sekwencja sie zgadza i osiagnelismy 7 na counterze
		  		* to konami code zostal wpisany
		  		*/
		  		else if(counter == 7){
		  			//informujemy watek wypisujacy, ze podano sekwencje		  			
		  			konami_code_entered = true;                   		   
		  			printk("Konami entered!!!!!!\n");	
		  		}
		  				  				  	     
		  		counter++;
		  		if(counter == 8) 
		  			counter = 0;		  				 
          		}
          		
          		else{
          		
          			if(param->value == KEY_S)		
		  			konami_code_entered = false;
          			
          			//reset sekwencji jesli wpisano cos innego niz strzalke          			
          			for(int i = 0; i < 8; i++)
						konami[i] = 0;
					counter = 0;
					printk("Resetting sequence\n");
          		}

        	}
        	
    	}

    	return NOTIFY_OK;
}



static struct notifier_block keyboard_nb = {
    	.notifier_call = keyboard_interrupt_handler
};

//funckja to inicjalizacji urzadzenia wejsciowego
static void init_input_device(void)
{
	int result;
	//Alokacja struktury urzadzenia wejsciowego
	input_device = input_allocate_device();
	if (!input_device) {
		printk(KERN_ERR "Failed to allocate input device\n");
		return;
	}

	//Wlasciwosci urzadneia
	input_device->name = "Virtual Keyboard";
	input_device->id.bustype = BUS_VIRTUAL;
	input_device->evbit[0] = BIT_MASK(EV_KEY);

	//Wlaczenie odpowiednich klawiszy
	//linia 1
    	set_bit(KEY_SLASH, input_device->keybit);
	set_bit(KEY_BACKSLASH, input_device->keybit);
	set_bit(KEY_MINUS, input_device->keybit);

	//linia 2
	set_bit(KEY_KPLEFTPAREN, input_device->keybit);
	set_bit(KEY_SPACE, input_device->keybit);
	set_bit(KEY_O, input_device->keybit);
	set_bit(KEY_DOT, input_device->keybit);
	set_bit(KEY_KPRIGHTPAREN, input_device->keybit);

	//linia 3
	set_bit(KEY_SPACE, input_device->keybit);
	set_bit(KEY_DOT, input_device->keybit);   
	set_bit(KEY_COMMA, input_device->keybit);      
	set_bit(KEY_6, input_device->keybit);   
	set_bit(KEY_LEFTSHIFT, input_device->keybit);       
	
	//Enter
	set_bit(KEY_ENTER, input_device->keybit);

	//Rejestracja urzadneia
	result = input_register_device(input_device);
}

//funckja inicjalizacji modulu
int init_module()
{
    	init_input_device();
    	register_keyboard_notifier(&keyboard_nb);
    	//tworzymy watek konami
    	konami_thread = kthread_create(display_easter_egg, NULL, "konami_thread");
    	if (IS_ERR(konami_thread)) {
       		printk(KERN_ERR "Failed to create konami_thread\n");
        	return PTR_ERR(konami_thread);
    	}
    
    	printk("Konami listener initialized\n");
    	//rozpoczynamy proces konami
    	wake_up_process(konami_thread);  
     
    	return 0;
}

//funkcja czyszcaca modul po wyladowaniu
void cleanup_module()
{
    	unregister_keyboard_notifier(&keyboard_nb);
    
    	if (input_device) {
        	input_unregister_device(input_device);
        	input_free_device(input_device);
        	input_device = NULL;
    	}
    
    	//zakonczenie watku zewnetrzengo
    	if (konami_thread) {
		kthread_stop(konami_thread);
		pr_info("Konami thread stopped\n");
    	}
     
    	printk("Konami listener unloaded\n");
}

