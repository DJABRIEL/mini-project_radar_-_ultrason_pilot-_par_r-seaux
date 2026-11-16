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
 *      Servo : TIM5_CH4 (PI0), 50 Hz, 1 tick = 0.5 µs (PSC = 49, horloge 100 MHz)
 *      1000 µs = 2000 ticks (0°), 2000 µs = 4000 ticks (180°)
 *---------------------------------------------------------------------------*/
#define SERVO_CCR_MIN  2500
#define SERVO_CCR_MAX  5000
#define SWEEP_MAX_DEG 180
int      current_angle = 0;        /* lue par UDP_Socket.c (ne pas mettre static) */
volatile int sweep_enabled = 1;    /* mise à 0 par UDP_Socket.c quand une commande arrive */
volatile int distance_cm_value = 0;   /* mise à jour par la tâche du capteur */

osThreadId_t tid_Test;
void PG6_Interrupt_Init(void);
void EXTI9_5_IRQHandler(void);

/* Fonction non bloquante : utilisable depuis une tâche ou un callback */

void servo_set_angle(int deg)
{
	if (deg < 0)   deg = 0;
	if (deg > 180) deg = 180;
	
	current_angle = deg;          /* mémorise l'angle pour la réponse UDP */
	int val =300- deg *600/180;
	TIM5->CCR4 = SERVO_CCR_MIN + deg * (SERVO_CCR_MAX - SERVO_CCR_MIN) / 180;
}

/*----------------------------------------------------------------------------
 *      Tâche de test : balayage du servo 0 -> 180 -> 0
 *---------------------------------------------------------------------------*/
static void Thread_Test (void *argument)
{
  (void)argument;

  /* Activation des horloges GPIO */
  RCC->AHB1ENR |= (1 << 8);            // GPIOI
  RCC->AHB1ENR |= (1 << 5);            // GPIOF
  RCC->AHB1ENR |= (1 << 0);            // GPIOA

  /* ---- TIM10_CH1 sur PF6 (AF3) : 10 µs, 50 % ---- */
  RCC->APB2ENR |= (1 << 17);           // TIM10
  GPIOF->MODER |= (2 << 12);           // PF6 en mode AF
  GPIOF->AFR[0] |= (3 << 24);          // PF6 = AF3 (TIM10_CH1)
  TIM10->CCMR1 |= (6 << 4);            // PWM mode 1 sur CH1
  TIM10->CCMR1 |= (1 << 3);            // OC1PE : preload
  TIM10->CCER  |= (1 << 0);            // CC1E : sortie CH1 active
  TIM10->PSC  = 0;
  TIM10->ARR  = 1999;                  // 2000 x 5 ns = 10 µs (suppose horloge 200 MHz)
  TIM10->CCR1 = 1000;
  TIM10->EGR |= (1 << 0);              // UG : charge PSC/ARR
  TIM10->CR1 |= (1 << 0);

  /* ---- TIM5_CH4 sur PI0 (AF2) : servo 50 Hz ---- */
  RCC->APB1ENR |= (1 << 3);            // TIM5
  GPIOI->MODER |= (2 << 0);            // PI0 en mode AF
  GPIOI->AFR[0] |= (2 << 0);           // PI0 = AF2 (TIM5_CH4)
  TIM5->CCMR2 |= (6 << 12);            // PWM mode 1 sur CH4
  TIM5->CCMR2 |= (1 << 11);            // OC4PE : preload
  TIM5->CCER  |= (1 << 12);            // CC4E : sortie CH4 active
  TIM5->PSC  = 49;                     // tick 0,5 µs si horloge 100 MHz
  TIM5->ARR  = 39999;                  // 40000 x 0,5 µs = 20 ms = 50 Hz
  TIM5->CCR4 = 3000;                   // 1500 µs (90°) au départ
  TIM5->EGR |= (1 << 0);               // UG : charge PSC/ARR
  TIM5->CR1 |= (1 << 0);

  /* ---- DAC sortie PA4 (inchangé) ---- */
  RCC->APB1ENR |= (1 << 29);           // DAC
  GPIOA->MODER |= (3 << 8);            // PA4 en analogique
  DAC->CR      |= (1 << 0);            // EN1

  /* ---- Balayage du servo ---- */
  int angle = 0;
  int step  = 2;

  for (;;)
  {
		if (sweep_enabled)              /* pas de balayage pendant une commande UDP */
    {
    servo_set_angle(angle);
    /* Afficher l'angle sur l'écran LCD ici (et la distance si besoin, ex. distance_cm()) */

    angle += step;
    if (angle >= 180)     { angle = 180; step = -2; }
    else if (angle <= 0)  { angle = 0;   step = 2;  }
		}
    osDelay(50);                       /* 2° toutes les 50 ms */
	
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

/*----------------------------------------------------------------------------
 *      main
 *---------------------------------------------------------------------------*/
int main(void)
{
  configuration_overclock();           // pour pouvoir utiliser l'écran LCD

  /**********************************
  Début Initialisation périphérique
  **********************************/

  // Initialiser vos périphériques ici
 Init_Ethernet();
	PG6_Interrupt_Init();
  /**********************************
  Fin Initialisation périphérique
  **********************************/

  /**********************************
  Utilisation RTOS (Keil RTX5)
  **********************************/
#ifdef RTE_CMSIS_RTOS2
  /* Initialize CMSIS-RTOS2 */
  osKernelInitialize();

  /* Create thread functions that start executing */
 
  Init_GUIThread();
  Init_TestThread();                   // tâche de test (boucle avec osDelay)

  /* Start thread execution */
	
	
		
  osKernelStart();
#endif

  /* Infinite loop (ne s'exécute pas avec le RTOS !) */
  while (1)
  {
  }
}

void PG6_Interrupt_Init(void)
	{
		/* Horloges */
		RCC->AHB1ENR |= (1 << 6);            // GPIOG
		RCC->AHB1ENR |= (1 << 8);            // GPIOI (pour la sortie PI1)
		RCC->APB2ENR |= (1 << 14);           // SYSCFG

		/* PG6 en entrée, PI1 en sortie */
		GPIOG->MODER &= ~(3 << 12);          // PG6 : entrée
		GPIOI->MODER &= ~(3 << 2);
		GPIOI->MODER |=  (1 << 2);           // PI1 : sortie
		
		RCC->APB1ENR |= (1 << 0);             // horloge TIM2
		TIM2->PSC = 99;                       // 100 MHz / 100 = 1 MHz -> 1 tick = 1 µs
		TIM2->ARR = 0xFFFFFFFF;               // compteur 32 bits
		TIM2->EGR |= (1 << 0);                // charge PSC et ARR

		/* Relier la ligne EXTI6 au port G */
		SYSCFG->EXTICR[1] &= ~(0xF << 8);
		SYSCFG->EXTICR[1] |=  (6 << 8);      // 6 = port G

		/* Front montant et descendant, ligne 6 démasquée */
		EXTI->RTSR |= (1 << 6); // PG6 : on réagit quand il passe de 0 à 1
		EXTI->FTSR |= (1 << 6); // PG6 : on réagit aussi quand il passe de 1 à 0
		EXTI->IMR  |= (1 << 6); // on autorise la ligne 6 à envoyer des interruptions

		/* NVIC */
		NVIC_SetPriority(EXTI9_5_IRQn, 5);
		NVIC_EnableIRQ(EXTI9_5_IRQn);
	}
	
	

void EXTI9_5_IRQHandler(void)
{
	volatile int tick_value;      // valeur lue par le thread

	
  if (EXTI->PR & (1 << 6))
  {
    EXTI->PR = (1 << 6);               // effacer le drapeau (écrire 1)

    if (GPIOG->IDR & (1 << 6))         // IDR : PG6 = 1 -> front montant
    {
      GPIOI->BSRR = (1 << 1);        	// PI1 à 1                
      TIM2->CR1 &= ~(1 << 0);
      TIM2->CNT = 0;
      TIM2->CR1 |= (1 << 0);           // démarrage du timer
    }
    else                               // IDR : PG6 = 0 -> front descendant
    {
      TIM2->CR1 &= ~(1 << 0);          // arrêt du timer
      tick_value = TIM2->CNT;          // récupération de la valeur
			distance_cm_value = tick_value/58;  // son : environ 58 µs par cm aller-retour

      GPIOI->BSRR = (1 << (1 + 16));   // BSRR : PI1 à 0
    }
  }
}