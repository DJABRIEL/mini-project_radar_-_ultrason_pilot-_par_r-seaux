#include "stm32f7xx.h"                  // Device header

extern void configuration_overclock(void);
extern int Init_GUIThread (void);
extern void Init_Ethernet (void);
#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif
#ifdef RTE_CMSIS_RTOS2                  // when RTE component CMSIS RTOS2 is used
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif

/*----------------------------------------------------------------------------
 *      Tâche de test (vide) : à compléter par les étudiants
 *---------------------------------------------------------------------------*/
osThreadId_t tid_Test;


static void Thread_Test (void *argument)
{
  (void)argument;

  // Initialisations propres à la tâche (si besoin) ici
	RCC ->AHB1ENR |= (1 << 8);//activation GPIOI
	GPIOI->MODER |= (1 << 2);//mode GPIOI1 
	GPIOI->MODER |= (1 << 0);//mode GPIOI  	
	
	//activation DAC
	RCC ->APB1ENR |= (1 << 29);//activation DAC
	RCC ->AHB1ENR |= (1 << 0);// activation GPIOA
	GPIOA->MODER 	|= (3<<8);//activation PA4
	DAC ->CR 			|= (1<<0);//mode sortie pa4 
	
	//activation PWM
	RCC->APB1ENR |= (1 << 3);     // Activation TIM5
	GPIOI->MODER |= (2 << 0);     // PI0 en mode Alternate Function (AF)
	GPIOI->AFR[0] |= (2 << 0);    // PI0 en mode AF2 (TIM5_CH1)
	
	TIM5->CCMR1 |= (6 << 4);//TIM5 in PWM mode 1
	TIM5->CCER |= (1 << 0);//connect OC1 to TIM5_CH1
	TIM5->PSC = 49;             // Prédiviseur (100 MHz / 50 = 2 MHz)
	TIM5->ARR = 39999;          // Auto-reload : période de 20 ms (2 MHz / 40000 = 50 Hz)

	TIM5->CR1 |= (1 << 0);      // Démarrage du Timer 5
	
	
	
	

  while (1)
  {
    // Votre boucle de test ici (mesure de distance, balayage du servo, etc.)
				

    osDelay(100);   // période de la boucle : 100 ms
  }
}

static int Init_TestThread (void)
{
  tid_Test = osThreadNew(Thread_Test, NULL, NULL);
  if (tid_Test == NULL) {
    return -1;
  }
  return 0;
}


int main(void)
{
  configuration_overclock(); //pour pouvoir utiliser l'écran LCD

	/**********************************
	Début Initialisation périphérique
	**********************************/

	//Initialiser vos périphériques ici

	/**********************************
	Fin Initialisation périphérique
	**********************************/

	/**********************************
	Utilisation RTOS (Keil RTX5)
	**********************************/
	#ifdef RTE_CMSIS_RTOS2
	  /* Initialize CMSIS-RTOS2 */
	  osKernelInitialize ();

	  /* Create thread functions that start executing */
		Init_Ethernet();
		Init_GUIThread();
		Init_TestThread();   // tâche de test (boucle avec osDelay)

	  /* Start thread execution */
	  osKernelStart();
	#endif

  /* Infinite loop  (ne s'exécute pas avec le RTOS!!!)*/
  while (1)
  {
  }
}