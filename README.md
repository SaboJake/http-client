# Tema 4. Client web. Comunicație cu REST API
Lupoi Ștefan-Alexandru 325CA

## 1. Descriere
* Această temă implementează un client HTTP în C++ ce comunică cu un server
* Clientul are la dispoziție diverse comenzi pentru managemnet-ul de filme dintr-o bibliotecă
* Există și utilizatori admin ce pot adăuga/șterge alți utilizatori
* Datele primitite și trimise de client către server sunt în format JSON

## 2. JSON (nlohmann)
* După cum am precizat date precum numele de utilizatori, filme, etc. sunt prelucrate in format JSON
* Pentru parsarea datelor de format JSON am decis să folosesc biblioteca nlohmann
* Aceasta se remarcă prin simplitatea utilizării:
    * Accesul și modificarea câmpurilor este facilă
    * Sintaxa este modernă, în stil puternic C++

## 3. Comunicare cu serverul
* Pentru a trimite/primi mesaje, clientul deschide un unic socket
* Trimiterea/primirea mesajelor în intregime este asigurată de scrierea/citirea într-un loop
* Primirea unui mesaj se realizează astfel:
    * Se citește headerul HTTP
    * Se extrage ”Content-Length”
    * Se citește body-ul și se construiește răspunsul

## 4. Requests
* Pentru realizarea operațiilor, clientul trimite 4 tipuri de requesturi către server:
    * GET - primire informații de pe server (ex: informație film)
    * POST - adaugare informații noi pe server (ex: adaugare filme)
    * DELETE - ștergere informație de pe server (ex: ștergere filme)
    * PUT - înlocuire informație de pe server (ex: update film)
* Acestea pot include și cookie-uri și token-ul JWT
* Sunt create cu ajutorul funcțiilor corespunzătoare

## 5. Funcționalitate
* Clientul primește comenzi de la tastatură, unele necesitând câmpuri adiționale
* În urma executării comenzii, acesta poate răspunde cu ”SUCCESS” sau ”ERROR”
* Pentru executarea unei comenzi sunt trimise unul sau mai multe requesturi către server
* Corectitudinea comenzilor cât și a câmpurilor este realiztă corespunzător
* Funcția `input_check` alături de enum-ul `restriction` verifică inputul și generează mesaje de eroare
* Înainte de executarea unei comenzi mai sunt verificate și următoarele aspecte:
    * Dacă este o comandă de tip admin (add_user, get_users, etc.) se verifică dacă un admin este logat
    * Dacă este o comandă de tip normal (get_movie, add_movie, etc.) se verifică dacă utilizatorul are access la bibliotecă
* Accesul unui utilizator la biblioteca de filme este realizat prin intermeduiul unui JWT token
* Utilizatorii pot primi un token de la server apelând comanda get_access
* Răspunsurile primite de la server sunt verificate
* Ex: comanda get_movies verifică dacă răspunsul conține câmpul ”movies” și dacă acesta este un array

## 6. Lambdauri. Funcționale
* Corespondența dintre comenzi și executarea lor este realizată de un unordered map:
    * cheie: numele comenzii
    * valoare: funcție void ce ia ca parametru un lambda
* În cazul în care sunt necesari niște flaguri în plus, valoarea este un alt lambda

```cpp
command_map["add_movie"] = [](make_request_type make_request) {
        add_or_update_movie(make_request, false);
};
```
* Comenzile de login sunt tratate separat (nu utilizează `make_request`)
* În cadrul funcției main se remarcă lambdaul `make_request`:
    * Extrage corpul JSON cât și status code-ul din răspunsul de la server
    * Asigură integritatea răspunsului JSON
    * În caz de răspuns malformat sau inexistent se afișează eroare
    * De asemenea în caz de status code >= 400, afișează trunchiat eroarea extrasă din corpul JSON

## 7. Concluzie
* Interacțiunea cu un server ce nu se află pe mașina locală a fost interesantă
* Gestionarea comenzilor într-un mod corect, simplu și concis a fost dificilă
* Aplicarea noțiunilor din programarea funcțională (lambda) a simplificat codul
* Variabilele globale ar fi putu fi omise utilizând un struct trimis ca parametru
