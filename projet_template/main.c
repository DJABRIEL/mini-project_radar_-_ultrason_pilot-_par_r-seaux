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

void servo_set_angle(int deg){
//Val vaut une valeur entre -300 et 300
	int ccr_val;
	if (deg<0) deg=0;
	if (deg>180) deg=180;
  for (int i=0;i<181;i++) {
		deg = i;
	// Calcul du rapport cyclique basé sur ARR + 1 (soit 40000 pas au total)
	ccr_val = 1250 + deg * 5000/180; //ajout d'un offset 1250 pour faire un tour complet  
		// Modification du rapport cyclique en temps réel
		TIM5->CCR4 = ccr_val;
		 osDelay(30);   // période de la boucle : 100 ms
	}
	ccr_val = 0;
	
  //Val vaut une valeur entre -300 et 300
// Configuration de la période (à faire idéalement dans l'init, mais fonctionnel ici)


}

static void Thread_Test (void *argument)
{
  (void)argument;

  // Initialisations propres à la tâche (si besoin) ici
	RCC ->AHB1ENR |= (1 << 8);//activation GPIOI
	GPIOI->MODER |= (1 << 2);//mode GPIOI1 
	//GPIOI->MODER |= (1 << 0);//mode GPIOI0
	
	//activation PWM

		RCC ->APB1ENR |= (1 << 3);//activation TIM5
		GPIOI->MODER |= (2 << 0);//PI0 in AF mode
		GPIOI->AFR[0] |= (2 << 0);//PI0 in AF2 mode (TIM5_CH4)
		TIM5->CCMR2 |= (6 << 12);//TIM5 in PWM mode 1
		TIM5->CCER |= (1 << 12);//connect OC1 to TIM5_CH4
		//parametre 50Hz
		TIM5->PSC = 49;
    TIM5->ARR = 39999		;
		TIM5->CCR4 = 20000;
		TIM5->CR1 |= (1<<0);

	
	
	//activation DAC
	RCC ->APB1ENR |= (1 << 29);//activation DAC
	RCC ->AHB1ENR |= (1 << 0);// activation GPIOA
	GPIOA->MODER 	|= (3<<8);//activation PA4
	DAC ->CR 			|= (1<<0);//mode sortie pa4 
	
	

  while (1)
  {
    // Votre boucle de test ici (mesure de distance, balayage du servo, etc.)
		//distance_cm(void);
		servo_set_angle(-200);

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