# TPsynthese : Client-TFTP

Rayen BEN AMAR

Lotfi HENNI

### Question 1 : 
      Nous récupérons l'adresse IP, le port et le nom de fichier à partir des arguments.
      Exemple "./gettftp 127.0.0.1 69 ensea.png" nous récupérons l'adresse IP du serveur, le port et le nom du fichier dans des constantes.
      Nous avons fais un test sur le nombre d'argument reçu, si l'on reçois moins de 3 arguments en entré une erreur s'affichera et le programme s'arrêtera.


### Question 2 : 
      Nous avons déclaré deux structures de type addrinfo hints et res.
      hints est une structure qui servira à spécifier les critères pour la recherche d'informations sur l'adresse.
      res sera utilisé pour stocker les résultats de cette recherche.
      Puis on fera appel à la fonction getaddrinfo pour récupérer les informations sur le serveur "Adresse IP" et le "Port".
      Un test a été fait là aussi si la fonction getaddrinfo retourne une valeur différente de 0 un message d'erreur sera affiché.


### Question 3 : 
      En utilisant les informations récupéré grâce à la fonction getaddrinfo nous allons reservé un socket de connexion vers le serveur TFTP.
      La fonction utilisé pour reservé ce socket est socket().
      Là aussi nous faisons un test si la fonction retourne un nombre inférieur à 0 un message d'erreur sera affiché.


### Question 4-a : 
      Pour la création de tous nos paquets lors de ce TP "RRQ, WRQ, DAT et ACK" nous avons choisis de travaillé avec une structure déclaré au début nommé TFTP_Packet
      Nous utiliserons tout au long de la suite de ce tp deux fonction de la librairie <arpa/inet.h> qui sont <htons()> et <ntohs()>.
      <htons()> est utilisé pour convertir un short depuis l'ordre des octets de la machine vers l'ordre des octets du réseau (pour formater nos paquets).
      <ntohs()> est utilisé pour convertir un short depuis l'ordre des octets du réseau vers l'ordre des octets de la machine (pour effectuer nos différents tests).
      Pour le RRQ nous utilisons notre structure précédemment déclaré l'opcode est initialisé à htons(1).
      Nous formaterons la suite de notre paquet RRQ avec le nom de fichier, 0, le mode "octet", 0.
      Puis nous l'envoyons à notre serveur TFTP à l'aide de la fonction sendto().

### Question 4-b :
     Nous créerons un fichier à l'aide de la fonction fopen() avec le mode "wb" qui signifie mode "w: écriture", "b: binaire".
     Nous utiliserons la fonction recvfrom() pour réceptionner notre paquet de donnée dans un paquet DAT formaté à l'aide de notre structure TFTP_Paquet avec un opcode de 3.
     Puis nous écrirons les données reçus depuis notre paquet DAT sur notre fichier créer précédemment avec la fonction fwrite().
     Plusieurs tests et printf() sont mis en place pour mieux analyser et débugger notre code.

