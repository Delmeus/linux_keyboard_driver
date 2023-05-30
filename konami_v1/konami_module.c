#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/keyboard.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Delmeus, okejka1, asiazo");
MODULE_DESCRIPTION("Konami code listener");

#define KEYBOARD_IRQ 1  // Numer przerwania klawiatury
#define KEY_UP 103      // Kod klawisza dla strzałki w górę
#define KEY_DOWN 108
#define KEY_RIGHT 106
#define KEY_LEFT 105
#define DELAY_MS 160   // Opóźnienie w milisekundach

static unsigned long last_key_time = 0;

//tablica do sprawdzania stanu konami code
static int konami[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static int counter = 0;

static int lastCode;

static int keyboard_interrupt_handler(struct notifier_block *nblock, unsigned long code, void *_param)
{
    struct keyboard_notifier_param *param = _param;
    unsigned long current_time = jiffies_to_msecs(jiffies);

	if (code == KBD_KEYCODE ) {
	
		if (current_time - last_key_time >= DELAY_MS) {
			if(current_time - last_key_time > 3000){
				for(int i = 0; i < 8; i++)
					konami[i] = 0;
				counter = 0;
			}
			
			lastCode = param->value;      
            		last_key_time = current_time;
            
          		
          		konami[counter] = lastCode;
          	         			
          		if(konami[0] == KEY_UP && konami[1] == KEY_UP && konami[2] == KEY_DOWN && konami[3] == KEY_DOWN 
          		&& konami[4] == KEY_LEFT && konami[5] == KEY_RIGHT && konami[6] == KEY_LEFT && konami[7] == KEY_RIGHT){
          			printk("WPROWADZONO KONAMI!!!!!!\n");
          		}
          			
          		printk("Ostatni kod przycisku %d", lastCode);
          	     
          		counter++;
          		if(counter == 8) 
          			counter = 0;
          			
          		if(lastCode != KEY_UP && lastCode != KEY_DOWN && lastCode != KEY_LEFT && lastCode != KEY_RIGHT){
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
    //printk("Wciśnięto coś\n");
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

