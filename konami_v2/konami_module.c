#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/keyboard.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Delmeus, okejka1, asiazo");
MODULE_DESCRIPTION("Konami code listener");

#define KEYBOARD_IRQ 1  // Numer przerwania klawiatury
#define KEY_UP 103      // Kod klawisza dla strzałki w gore
#define KEY_DOWN 108	// Kod klawisza dla strzałki w dol
#define KEY_RIGHT 106	// Kod klawisza dla strzałki w prawo
#define KEY_LEFT 105	// Kod klawisza dla strzałki w lewo
#define DELAY_MS 160   	// Opóźnienie w milisekundach

static unsigned long last_key_time = 0;

//tablica do sprawdzania stanu konami code
static int konami[8] = {0, 0, 0, 0, 0, 0, 0, 0};
//oczekiwana sekwencja
static int expected_seq[8] = {KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT};

static int counter = 0;

static int lastCode;

static int keyboard_interrupt_handler(struct notifier_block *nblock, unsigned long code, void *_param)
{
    struct keyboard_notifier_param *param = _param;
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
		    		  				  				  	
		  		if(konami[0] == KEY_UP && konami[1] == KEY_UP && counter == 2 && lastCode == KEY_UP){
		  			counter = 1;
		  		}
		  		
		  		
		  		konami[counter] = lastCode;
		  		printk("Konami[%d] = %d", counter, lastCode);
		  		
		  		if(konami[counter] != expected_seq[counter]){
		  			//reset sekwencji jesli nie zgadza sie z oczekiwana
		  			for(int i = 0; i < 8; i++)
						konami[i] = 0;
					//counter na -1 poniewaz zaraz zwiekszamy go o 1
					counter = -1;
					printk("Resetuje sekwencje\n");
		  		}
		  		//jezeli sekwencja sie zgadza i osiagnelismy 7 na counterze
		  		//to konami code zostal wpisany
		  		else if(counter == 7)
		  			printk("WPROWADZONO KONAMI!!!!!!\n");
		  		
		  		
		  	     
		  		counter++;
		  		if(counter == 8) 
		  			counter = 0;
		  				 
          		}
          		else{
          			//reset sekwencji jesli wpisano cos innego niz strzalke
          			for(int i = 0; i < 8; i++)
						konami[i] = 0;
					counter = 0;
					printk("Resetuje sekwencje\n");
          		}
          		
        }
    }

    return NOTIFY_OK;
}



static struct notifier_block keyboard_nb = {
    .notifier_call = keyboard_interrupt_handler
};

static irqreturn_t keyboard_interrupt(int irq, void *dev_id)
{
    return IRQ_HANDLED;
}

int init_module()
{
    int result;
    register_keyboard_notifier(&keyboard_nb);
    
    printk("Konami listener initialized\n");
    result = request_irq(KEYBOARD_IRQ, keyboard_interrupt, IRQF_SHARED, "keyboard_module", (void *)keyboard_interrupt);
    if (result != 0) {
        printk(KERN_ERR "Błąd podczas rejestracji obsługi przerwania klawiatury\n");
        return result;
    }
    return 0;
}


void cleanup_module()
{
    unregister_keyboard_notifier(&keyboard_nb);
    free_irq(KEYBOARD_IRQ, (void *)keyboard_interrupt);
    printk("Konami listener unloaded\n");
}

