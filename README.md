# Rush DNSD - Groupe 8

Rush afin d'implémenter un DNS en C.

## Conventions
- Commentaires: Anglais
- README: Français

### Tasks

- [x] Prototype
- [x] Répartition tasks (samedi 8h30)
- [x] Parsing JSON
- [x] Implémentation dns_entry
- [x] Implémentation hashmap
- [x] Parsing des requetes:
    - [x] TCP / UDP (L4) (Ipv4/Ipv6)
    - [x] DNS (L7)
- [x] Construction epoll server
- [x] Implémentation réponses:
    - [x] DNS (L7)
    - [x] TCP / UDP (L4) (Ipv4/Ipv6)

### How to use

Afin de build le projet:
```
make && make install
```
Afin de lancer le serveur:
```
./build/<MODE>/dnsd -f <zone_config_file> -<ipvX_number> <server_ip>:<port>
```
Afin de vérifier la validité du fichier de configuration il faut utiliser l'option '-c'

### Fuzzing

Un fuzzer a été lancé sur notre serveur. Il nous a permi de détecter des problèmes mémoires lors du parsing TCP.

Afin de le lancer (le serveur doit être actif) :
```
sudo python fuzz.py -i <server_ip> -p <port>
```
Pour un fuzzer en tcp, l'option '-t' existe.
