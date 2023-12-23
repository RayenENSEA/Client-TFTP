# TPsynthese : Client-TFTP

Rayen BEN AMAR

Lotfi HENNI

### Question 1 : 
      Nous récupérons l'adresse IP, le port et le nom de fichier à partir des arguments.
      Exemple "./gettftp 127.0.0.1 69 ensea.png" nous récupérons l'adresse IP du serveur, le port et le nom du fichier dans des constantes.
      Nous avons fait un test sur le nombre d'arguments reçu, si l'on reçoit moins de 3 arguments en entrée une erreur s'affichera et le programme s'arrêtera.


### Question 2 : 
      Nous avons déclaré deux structures de type addrinfo hints et res.
      hints est une structure qui servira à spécifier les critères pour la recherche d'informations sur l'adresse.
      res sera utilisé pour stocker les résultats de cette recherche.
      Puis on fera appel à la fonction getaddrinfo pour récupérer les informations sur le serveur "Adresse IP" et le "Port".
      Un test a été fait là aussi si la fonction getaddrinfo retourne une valeur différente de 0 un message d'erreur sera affiché.


### Question 3 : 
      En utilisant les informations récupérées grâce à la fonction getaddrinfo nous allons reserver un socket de connexion vers le serveur TFTP.
      La fonction utilisée pour reserver ce socket est socket().
      Là aussi, nous faisons un test si la fonction retourne un nombre inférieur à 0 un message d'erreur sera affiché.


### Question 4-a : 
      Pour la création de tous nos paquets lors de ce TP "RRQ, WRQ, DAT et ACK" nous avons choisi de travailler avec une structure déclarée au début nommée TFTP_Packet
      Nous utiliserons tout au long de la suite de ce tp deux fonctions de la librairie <arpa/inet.h> qui sont <htons()> et <ntohs()>.
      <htons()> est utilisé pour convertir un short depuis l'ordre des octets de la machine vers l'ordre des octets du réseau (pour formater nos paquets).
      <ntohs()> est utilisé pour convertir un short depuis l'ordre des octets du réseau vers l'ordre des octets de la machine (pour effectuer nos différents tests).
      Pour le RRQ, nous utilisons notre structure précédemment déclarée avec l'opcode qui est initialisé à htons(1).
      Nous formaterons la suite " content " de notre paquet RRQ avec le nom de fichier, 0, le mode "octet", 0 à l'aide de la fonction sprintf.
      Puis nous l'envoyons à notre serveur TFTP à l'aide de la fonction sendto().

### Question 4-b :
     Nous créerons un fichier à l'aide de la fonction fopen() avec le mode "wb" qui signifie "w: écriture", "b: binaire".
     Nous utiliserons la fonction recvfrom() pour réceptionner notre paquet de données dans un paquet DAT formaté à l'aide de notre structure TFTP_Paquet avec un opcode de 3.
     Puis nous écrirons les données reçues depuis notre paquet DAT sur notre fichier créer précédemment avec la fonction fwrite().
     Enfin, nous créons un paquet ACK avec un opcode de 4 et le numéro de block qu'on aurait récupéré du paquet DAT précédemment reçu et nous l'envoyons avec la fonction sendto().
     Plusieurs tests et printf() sont mis en place pour mieux analyser et débugger notre code.

### Question 4-c :
     Nous referons exactement ce qu'on a fait à la question 4-b, mais cela dans une boucle while(1) pour recevoir un fichier constitué de plusieurs paquets.
     Un test est fait à chaque fin de la boucle qui vérifie si le nombre de bits reçu n'est pas plus petit que 512 octets.
     "512 octets" est la taille maximale de chaque paquet.
     Si la taille d'un paquet est plus petit que 512 octets cela signifiera que c'est le dernier paquet et donc nous quitterons notre boucle.

### Question 5-a : 
      Pour le WRQ nous utilisons notre structure précédemment déclarée avec l'opcode qui est initialisé à htons(2).
      Nous formaterons la suite " content " de notre paquet WRQ avec le nom de fichier, 0, le mode "octet", 0 à l'aide de la fonction sprintf.
      Puis nous l'envoyons à notre serveur TFTP à l'aide de la fonction sendto().

### Question 5-b :
     Nous ouvrons le fichier a envoyé à l'aide de la fonction fopen() avec cette fois-ci le mode "rb" qui signifie "r: lecture", "b: binaire".
     Nous initialisons le block_num à 0.
     Nous créons le paquet DAT avec l'opcode qui est initialisé à htons(2).
     Puis avec la fonction fread() lirons exactement 512 octets du fichier à envoyé.
     Nous formaterons la suite " content " de notre paquet DAT avec le block_num et la data de 512 octets lu avec la fonction fread().
     Nous attendons par la suite le packet ACK reçu par le serveur TFTP, nous ferons un test dessus pour vérifier que le opcode de ce packet est bien à 4.

### Question 5-c :
     Nous referons exactement ce qu'on a fait à la question 5-b, mais cela dans une boucle while(1) pour envoyer un fichier constitué de plusieurs paquets.
     Un test est fait à chaque début de la boucle qui vérifie si le nombre de bits lu n'est pas égale à 0.
     Si le nombre de bits lu est égale à 0 cela signifiera qu'il ne reste plus rien n'a envoyé et donc nous quitterons notre boucle.

