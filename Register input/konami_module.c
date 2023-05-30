#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/keyboard.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Delmeus, okejka1, asiazo");
MODULE_DESCRIPTION("Konami code listener");

#define KEYBOARD_IRQ 1  // Numer przerwania klawiatury

static int keyboard_interrupt_handler(struct notifier_block *nblock, unsigned long code, void *_param)
{
    struct keyboard_notifier_param *param = _param;

    if (code == KBD_KEYCODE && param->value == K_UP) {
        // Tutaj możesz przetwarzać wciśnięcia klawiszy, np. sprawdzając sekwencje konami
    }

    return NOTIFY_OK;
}

static irqreturn_t keyboard_interrupt(int irq, void *dev_id)
{
	printk("Wcisnieto cos\n");
	return IRQ_HANDLED;
}

static struct notifier_block keyboard_nb = {
    .notifier_call = keyboard_interrupt_handler
};

int init_module()
{
	int result;
    	register_keyboard_notifier(&keyboard_nb);
    	printk("Konami listener initialized\n");
    	result = request_irq(KEYBOARD_IRQ, keyboard_interrupt, IRQF_SHARED,"keyboard_module", (void *)keyboard_interrupt);
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
