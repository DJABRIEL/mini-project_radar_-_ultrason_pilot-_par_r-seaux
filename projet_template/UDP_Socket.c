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

int32_t udp_sock;    //crée le sock                   // UDP socket handle
 
// Notify the user application about UDP socket events.
uint32_t udp_cb_func (int32_t socket, const  NET_ADDR *addr, const uint8_t *buf, uint32_t len) /* recevoir l'info de la carte */ {
	
  // Data received
   
    // IPv4 address: 192.168.0.154  Envoi vers la rasberry(le client)
    uint8_t *sendbuf;
		char str[32]; 	// On declare une chaine de caractere 
    sendbuf = netUDP_GetBuffer (len);
		// transformer le buffer recu en chaine de caractere 
		for(uint32_t i=0; i<len;i++)
		{	
			str[i] = buf[i];
		}
	 str[len] = '\0'; // caractère de fin de chaine indispensale pour le strcmp
		
		// Comparer le mot angle 
		if (strncmp(str,"angle",5)== 0 ){
			//Extrait la valeur après le mot angle 
			int angle = atoi(&str[5]);
			// code servomoteur
		}
		
		
		// Echo  a la rasberry 
		if (sendbuf != NULL){
		for(int i=0; i<len;i++){
			
			sendbuf[i] = buf[i];
		} 
		
			// repondre à l'expédidteur
		netUDP_Send(udp_sock, addr, sendbuf, len); 
   }
  
  return (0);
	
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

 // Pour voir que le message s'envoie bien en UDP vers ma carte a entrer sur la rasberry 
//! [code_UDP_Socket]
