#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/gpio.h>                 // Required for the GPIO functions
#include <linux/interrupt.h>            // Required for the IRQ code

MODULE_LICENSE("GPL");

static unsigned int gpioLED0 = 21;      // LED0 gpio (GPIO21)
static unsigned int gpioLED1 = 20;      // LED1 gpio (GPIO20)
static unsigned int gpioButton0 = 26;   // BUTTON0 gpio (GPIO26)
static unsigned int gpioButton1 = 19;   // BUTTON1 gpio (GPIO19)
static unsigned int gpioButton2 = 13;   // BUTTON2 gpio (GPIO13)
static unsigned int gpioButton3 = 5;    // BUTTON3 gpio (GPIO6)
static unsigned int irqNumber0;         // Used to share the IRQ number for button0
static unsigned int irqNumber1;         // Used to share the IRQ number for button1
static unsigned int irqNumber2;         // Used to share the IRQ number for button2
static unsigned int irqNumber3;         // Used to share the IRQ number for button3
static bool led0 = 0;                   // Is the LED0 on or off?
static bool led1 = 0;                   // Is the LED1 on or off?
static unsigned int presses0 = 0;       // Store the number of button0 presses
static unsigned int presses1 = 0;       // Store the number of button1 presses
static unsigned int presses2 = 0;       // Store the number of button2 presses
static unsigned int presses3 = 0;       // Store the number of button3 presses
static const char device_name[] = "Fase1";

static irq_handler_t fase1_irq_handler0(unsigned int irq0, void *dev_id, struct pt_regs *regs);
static irq_handler_t fase1_irq_handler1(unsigned int irq1, void *dev_id, struct pt_regs *regs);
static irq_handler_t fase1_irq_handler2(unsigned int irq2, void *dev_id, struct pt_regs *regs);
static irq_handler_t fase1_irq_handler3(unsigned int irq3, void *dev_id, struct pt_regs *regs);


// Registrem els GPIOss a fer servir
int register_device(void) {
    int result = 0;
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


    result = request_irq(irqNumber0,                                            // The interrupt number requested
                        (irq_handler_t) fase1_irq_handler0,                     // The pointer to the handler function below
                        IRQF_TRIGGER_RISING, "fase1_irq_handler0", NULL);
    printk(KERN_INFO "Fase1: El resultat de la interrupt request (device file) 0 es: %d\n", result);

    result = request_irq(irqNumber1,
                        (irq_handler_t) fase1_irq_handler1,
                        IRQF_TRIGGER_RISING, "fase1_irq_handler1", NULL);
    printk(KERN_INFO "Fase1: El resultat de la interrupt request (device file) 1 es: %d\n", result);

    result = request_irq(irqNumber2,
                        (irq_handler_t) fase1_irq_handler2,
                        IRQF_TRIGGER_RISING, "fase1_irq_handler2", NULL);
    printk(KERN_INFO "Fase1: El resultat de la interrupt request (device file) 2 es: %d\n", result);

    result = request_irq(irqNumber3,
                        (irq_handler_t) fase1_irq_handler3,
                        IRQF_TRIGGER_RISING, "fase1_irq_handler3", NULL);
    printk(KERN_INFO "Fase1: El resultat de la interrupt request (device file) 3 es: %d\n", result);

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




static irq_handler_t fase1_irq_handler0(unsigned int irq0, void *dev_id, struct pt_regs *regs){
   if (gpio_get_value(gpioButton0)) {
        led0 = true;                             // Turn on the LED0
        gpio_set_value(gpioLED0, led0);          // Set the physical LED accordingly
        printk(KERN_INFO "Fase1: Interrupt! Button 0 pressed\n");
        presses0++;                                 // Global counter, will be outputted when the module is unloaded
   }
   return (irq_handler_t) IRQ_HANDLED;
}

static irq_handler_t fase1_irq_handler1(unsigned int irq1, void *dev_id, struct pt_regs *regs) {
   if (gpio_get_value(gpioButton1)) {
        led0 = false;                            // Turn off the LED0
        gpio_set_value(gpioLED0, led0);          // Set the physical LED accordingly
        printk(KERN_INFO "Fase1: Interrupt! Button 1 pressed\n", gpio_get_value(gpioButton1));
        presses1++;
   }
   return (irq_handler_t) IRQ_HANDLED;
}

static irq_handler_t fase1_irq_handler2(unsigned int irq2, void *dev_id, struct pt_regs *regs) {
   if (gpio_get_value(gpioButton2)) {
        led1 = true;                             // Turn on the LED1
        gpio_set_value(gpioLED1, led1);          // Set the physical LED accordingly
        printk(KERN_INFO "Fase1: Interrupt! Button 2 pressed\n", gpio_get_value(gpioButton2));
        presses2++;
   }
   return (irq_handler_t) IRQ_HANDLED;
}

static irq_handler_t fase1_irq_handler3(unsigned int irq3, void *dev_id, struct pt_regs *regs) {
   if (gpio_get_value(gpioButton3)) {
        led1 = false;                            // Turn off the LED1
         gpio_set_value(gpioLED1, led1);          // Set the physical LED accordingly
         printk(KERN_INFO "Fase1: Interrupt! Button 3 pressed\n", gpio_get_value(gpioButton3));
         presses3++;
   }
   return (irq_handler_t) IRQ_HANDLED;

}




static int my_init(void) {
    printk(KERN_NOTICE "Fase1: Init LKM - Modul de la Fase1 engegat\n");
    register_device();
    return 0;
}

static void my_exit(void) {
    printk(KERN_INFO "Fase1: Has apretat els botons següents:\n\t- Botó 0: %d cops!\n\t- Botó 1: %d cops!\n\t- Botó 2: %d cops!\n\t- Botó 3: %d cops!\n", presses0, presses1, presses2, presses3);
    printk(KERN_NOTICE "Fase1: Exit LKM - Modul de la Fase1 apagat\n");
    unregister_device();
    return;
}

module_init(my_init);
module_exit(my_exit);
