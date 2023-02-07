# Network Monitor
System monitorowania parametrów sieci komputerowej w systemie operacyjnym GNU /Linux z użyciem gniazd sieciowych PF_NETLINK.
System wykrywa właczanie/wyłączanie interfejsów sieciowych, dodawanie/usuwanie w nich adresów IP, zmiany w tabeli routingów oraz pamięci podręcznej tablicy ARP.

Problem z wyswietlaniem adresów MAC oraz masek bitowych. (wszelkie flagi interfejsów są zapisane w maskach bitowych)
Trzeba było dodać w sumie 4 makra do obsługi 2 struktur. (ifinfomsg oraz ndmsg)

Plik do kompilacji nazywa się networkmonitor.c 
Kompilacja pliku: 
gcc -Wall ./networkmonitor.c -o ./networkmonitor
Uruchomienie pliku:
./networkmonitor
