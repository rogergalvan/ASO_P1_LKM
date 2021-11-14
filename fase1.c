// ASO : P1F1 - LKM
// https://github.com/rogergalvan/ASO_P1_LKM
// By: Roger Galvan (roger.galvan)
// 14/11/2021
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/gpio.h>                 // Required for the GPIO functions
#include <linux/interrupt.h>            // Required for the IRQ code

MODULE_LICENSE("GPL");

static unsigned int gpioLED0 = 21;              // LED0 gpio (GPIO21)
static unsigned int gpioLED1 = 20;              // LED1 gpio (GPIO20)
static unsigned int gpioButton0 = 26;           // BUTTON0 gpio (GPIO26)
static unsigned int gpioButton1 = 19;           // BUTTON1 gpio (GPIO19)
static unsigned int gpioButton2 = 13;           // BUTTON2 gpio (GPIO13)
static unsigned int gpioButton3 = 5;            // BUTTON3 gpio (GPIO6)
static unsigned int irqNumber0;                 // Used to share the IRQ number for button0
static unsigned int irqNumber1;                 // Used to share the IRQ number for button1
static unsigned int irqNumber2;                 // Used to share the IRQ number for button2
static unsigned int irqNumber3;                 // Used to share the IRQ number for button3
static bool led0 = 0;                           // Is the LED0 on or off?
static bool led1 = 0;                           // Is the LED1 on or off?
static int fuck_warnings;
static unsigned int presses[4] = {0,0,0,0};     // Store the number of button pressesf
static const char device_name[] = "Fase1";
static char* envp[] = {"HOME=/", NULL};
static char* argv0[] = {"/home/pi/Desktop/script0.sh", NULL};
static char* argv1[] = {"/home/pi/Desktop/script1.sh", NULL};
static char* argv2[] = {"/home/pi/Desktop/script2.sh", NULL};
static char* argv3[] = {"/home/pi/Desktop/script3.sh", NULL};




static irq_handler_t fase1_irq_handler (unsigned int irq, void *dev_id, struct pt_regs *regs);
static void ledAction(int ledN, bool onoff, int button);


// Registrem els GPIOss a fer servir
int register_device(void) {
    // Registrarem GPIOs
    if (!gpio_is_valid(gpioLED0) || !gpio_is_valid(gpioLED1)) {
        printk( KERN_WARNING "Fase1: Error al registrar els GPIO dels LEDs");
        return -ENODEV;
    }

    // Definim els estats inicials dels LEDs
    led0 = false;
    led1 = false;

    // Registrem els LEDs i polsadors
    // LED0
    gpio_request(gpioLED0, "sysf");
    gpio_direction_output(gpioLED0, led0);
    gpio_export(gpioLED0, false);
    // LED1
    gpio_request(gpioLED1, "sysf");
    gpio_direction_output(gpioLED1, led1);
    gpio_export(gpioLED1, false);

    // BUTTON0
    gpio_request(gpioButton0, "sysf");
    gpio_direction_input(gpioButton0);
    gpio_set_debounce(gpioButton0, 300);
    gpio_export(gpioButton0, false);
    // BUTTON1
    gpio_request(gpioButton1, "sysf");
    gpio_direction_input(gpioButton1);
    gpio_set_debounce(gpioButton1, 300);
    gpio_export(gpioButton1, false);
    // BUTTON2
    gpio_request(gpioButton2, "sysf");
    gpio_direction_input(gpioButton2);
    gpio_set_debounce(gpioButton2, 300);
    gpio_export(gpioButton2, false);
    // BUTTON3
    gpio_request(gpioButton3, "sysf");
    gpio_direction_input(gpioButton3);
    gpio_set_debounce(gpioButton3, 300);
    gpio_export(gpioButton3, false);


    // Check dels mappings dels IRQs
    irqNumber0 = gpio_to_irq(gpioButton0);
    printk(KERN_INFO "Fase1: El botó 0 esta mapejat al IRQ: %d\n", irqNumber0);
    irqNumber1 = gpio_to_irq(gpioButton1);
    printk(KERN_INFO "Fase1: El botó 1 esta mapejat al IRQ: %d\n", irqNumber1);
    irqNumber2 = gpio_to_irq(gpioButton2);
    printk(KERN_INFO "Fase2: El botó 2 esta mapejat al IRQ: %d\n", irqNumber2);
    irqNumber3 = gpio_to_irq(gpioButton3);
    printk(KERN_INFO "Fase3: El botó 3 esta mapejat al IRQ: %d\n", irqNumber3);

    fuck_warnings = request_irq(irqNumber0,
                        (irq_handler_t) fase1_irq_handler,
                        IRQF_TRIGGER_RISING, "fase1_irq_handler0", NULL);

    fuck_warnings = request_irq(irqNumber1,
                        (irq_handler_t) fase1_irq_handler,
                        IRQF_TRIGGER_RISING, "fase1_irq_handler1", NULL);

    fuck_warnings = request_irq(irqNumber2,
                        (irq_handler_t) fase1_irq_handler,
                        IRQF_TRIGGER_RISING, "fase1_irq_handler2", NULL);

    fuck_warnings = request_irq(irqNumber3,
                        (irq_handler_t) fase1_irq_handler,
                        IRQF_TRIGGER_RISING, "fase1_irq_handler3", NULL);

    return 0;
}



void unregister_device(void) {
    // Apaguem els LEDs i un unexport
    gpio_set_value(gpioLED0, 0);
    gpio_unexport(gpioLED0);
    gpio_set_value(gpioLED1, 0);
    gpio_unexport(gpioLED1);

    // Fem un free dels IRQs i un unexport dels botons
    free_irq(irqNumber0, NULL);
    gpio_unexport(gpioButton0);
    free_irq(irqNumber1, NULL);
    gpio_unexport(gpioButton1);
    free_irq(irqNumber2, NULL);
    gpio_unexport(gpioButton2);
    free_irq(irqNumber3, NULL);
    gpio_unexport(gpioButton3);

    // Fem un free de tots els GPIOs
    gpio_free(gpioLED0);
    gpio_free(gpioLED1);
    gpio_free(gpioButton0);
    gpio_free(gpioButton1);
    gpio_free(gpioButton2);
    gpio_free(gpioButton3);
}




static irq_handler_t fase1_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
    if (irq == irqNumber0) {
            ledAction(0, true, 0);
            //Execute script
            call_usermodehelper(argv0[0], argv0, envp, UMH_NO_WAIT);
    } else if (irq == irqNumber1) {
            ledAction(0, false, 1);
            //Execute script
            call_usermodehelper(argv1[0], argv1, envp, UMH_NO_WAIT);
    } else if (irq == irqNumber2) {
            ledAction(1, true, 2);
            //Execute script
            call_usermodehelper(argv2[0], argv2, envp, UMH_NO_WAIT);
    } else if (irq == irqNumber3) {
            ledAction(1, false, 3);
            //Execute script
            call_usermodehelper(argv3[0], argv3, envp, UMH_NO_WAIT);
    }
    return (irq_handler_t) IRQ_HANDLED;
}

static void ledAction(int ledN, bool onoff, int button) {
    if (ledN == 0) {
        led0 = onoff;
        gpio_set_value(gpioLED0, led0);
    } else if (ledN == 1) {
        led1 = onoff;
        gpio_set_value(gpioLED1, led1);
    }
    printk(KERN_INFO "Fase1: Interrupt! Button %d pressed\n", button);
    presses[button]++;  // Global counter, will be outputted when the module is unloaded
}






static int my_init(void) {
    printk(KERN_NOTICE "Fase1: Init LKM - Modul de la Fase1 engegat\n");
    register_device();
    return 0;
}

static void my_exit(void) {
    printk(KERN_INFO "Fase1: Has apretat els botons següents:\n\t- Botó 0: %d cops!\n\t- Botó 1: %d cops!\n\t- Botó 2: %d cops!\n\t- Botó 3: %d cops!\n", presses[0], presses[1], presses[2], presses[3]);
    printk(KERN_NOTICE "Fase1: Exit LKM - Modul de la Fase1 apagat\n");
    unregister_device();
    return;
}
module_init(my_init);
module_exit(my_exit);
