/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network:Service
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    UDP_Socket.c
 * Purpose: UDP Socket Code Template
 * Rev.:    V7.0.0
 *----------------------------------------------------------------------------*/
//! [code_UDP_Socket]
#include "rl_net.h"
#include "stm32f7xx.h"                  // Device header
#include "AppWizard.h"
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

/* Déclarés dans main.c */
extern void servo_set_angle(int deg);
extern int  current_angle;
extern volatile int sweep_enabled;
extern volatile int distance_cm_value;

int32_t udp_sock;    //crée le sock                   // UDP socket handle
 
// Notify the user application about UDP socket events.
uint32_t udp_cb_func (int32_t socket, const  NET_ADDR *addr, const uint8_t *buf, uint32_t len) /* recevoir l'info de la carte */ {
	
  // Data received
		char str[32]; 	// On declare une chaine de caractere 
		 // Trame vide ou trop longue : on ignore
		if (len == 0 || len >= sizeof(str)) {
			return 0;
		}
		 
    // IPv4 address: 192.168.0.154  Envoi vers la rasberry(le client)
    uint8_t *sendbuf;
		
    sendbuf = netUDP_GetBuffer (len);
		// transformer le buffer recu en chaine de caractere 
		for(uint32_t i=0; i<len;i++)
		{	
			str[i] = buf[i];
		}
	 str[len] = '\0'; // caractère de fin de chaine indispensale pour le strcmp
		
		// Comparer le mot angle 
		if (strncmp(str,"angle",5)== 0 ){
			 // Extrait la valeur après le mot "angle" (atoi ignore l'espace)
    int angle = atoi(&str[5]);
				sweep_enabled = 0;                  // arrête le balayage automatique
				servo_set_angle(angle);  						// commande le servo
				
  

			char txt[16];
			uint32_t n = (uint32_t)sprintf(txt, "angle %d\r\n", current_angle);
			uint8_t *sendbuf = netUDP_GetBuffer(n);
			
			if (sendbuf != NULL) {
      memcpy(sendbuf, txt, len); //memcpy(destination, source, nombre_d'octets)
      netUDP_Send(udp_sock, addr, sendbuf, len);
    }
  }

			// code servomoteur
	if (strncmp(str,"dist",4)== 0 ){
			sweep_enabled = 0;  
		
			char txt[16];	
			uint32_t n = (uint32_t)sprintf(txt, "distance %d\r\n", distance_cm_value);
			uint8_t *sendbuf = netUDP_GetBuffer(n);
		
		// Echo  a la rasberry 
	
		if (sendbuf != NULL) {
      memcpy(sendbuf, txt, n); //memcpy(destination, source, nombre_d'octets)
      netUDP_Send(udp_sock, addr, sendbuf, n);
    }
   
  
  return (0);
 }
	
}
 
// Send UDP data to destination client.
void send_udp_data (void) {

}
 
// Allocate and initialize the socket.
void Init_Ethernet (void) {
 
  netInitialize ();
 
  // Initialize UDP socket and open port 2000
  udp_sock = netUDP_GetSocket (udp_cb_func);
  if (udp_sock > 0) {
    netUDP_Open (udp_sock, 4000);  // On va utiliser le port 4000 pour la communication
  }
}

 // nc -u <ip> <port> Pour voir que le message s'envoie bien en UDP vers ma carte a entrer sur la rasberry 
//! [code_UDP_Socket]
