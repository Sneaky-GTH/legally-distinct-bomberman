"Legally Distinct Bomberman"

Autori:
 - Kārlis Čerņavskis, kc24020 (projektā ieguldīja 50%)
 - Gustavs Jānis Ozoliņš, go23005 (projektā ieguldīja 50%)

Pirmkods arī pieejams GitHub repozitorijā: https://github.com/Sneaky-GTH/legally-distinct-bomberman

Projekta pirmkods ir zem src direktorijas. Tas ir sadalīts apakšdirektorijās sekojoši:
 - src/client: Klients, kas tiek kompilēts kā ./build/client
 - src/server: Serveris, kas tiek kompilēts kā ./build/server
 - src/lib: Kopīgā bibliotēka, kas tiek izmantots gan serverī, gan klientā. Tiek kompilēts kā ./build/libbomberman.so

Kompilēšanai nepieciešams OpenGL (glut), pthreads un math bibliotēkas.

Pirms kompilēšanas Make izmanto FFMPEG, lai pārvērstu tekstūras no PNG uz RGBA formātu, ko mūsu spēle izmanto.
Tāpēc ir nepieciešams, lai FFMPEG būtu instalēts sistēmā, ja vēlas kompilēt projektu no nulles.
Šāds lēmums tika pieņemts, jo implementēt PNG dekodēšanu no nulles būtu pārāk laikietilpīgi, un FFMPEG ir plaši pieejams rīks, kas var veikt šo uzdevumu efektīvi.
ZIP failā ir iekļautas jau pārvērstās tekstūras, tāpēc FFMPEG nav nepieciešams, ja vien nevēlas veikt izmaiņas tekstūrās.

Spēles palaišanai ieteicams izmantot `make run_client` vai `make run_server` komandas, kas attiecīgi palaidīs klientu vai serveri.
Serveris pēc noklusējuma klausās portā 12345.

Klients ir spējīgs pievienoties gan caur IPv4, gan IPv6 adresēm, un var translēt hostvārdus uz IP adresēm, izmantojot getaddrinfo funkciju.

Kopīgā bibliotēka tiek dinamiski saistīta (to neiekļauj kompilētajā serverī un klientā).
Centāmies atšifrēt projekta protokolu un pie tā pieturēties. Diemžēl, ar doto protokolu nav iespējams pilnībā implementēt spēles
nosacījumus, piemēram, laukuma izvēli.

Minimālais palaišanas komplekts ir ar šādu direktoriju:
 - `client` vai `server` programma
 - `libbomberman.so` kopīgā bibliotēka
 - `assets/` (ja izmanto klientu)
   - `font.rgba`
   - `font.txt`
   - `world_tileset.rgba`
Klients meklēs `assets` direktoriju tekošajā direktorijā.

Klientam spēles laikā ir 3 pavedieni:
Oriģinālais paliek kā renderēšanas pavediens, otrs ir spēles loģikas pavediens, kas apstrādā spēles notikumus un atjaunina spēles stāvokli,
un trešais ir tīkla pavediens, kas sazinās ar serveri un saņem spēles notikumus.
Spēles loģikas pavediens tiek izveidots programmas sākumā, un tās stāvoklis katru reizi tiek notīrīts, kad izveido jaunu savienojumu.
Komunikācija starp pavedieniem tiek veikta, izmantojot pthread_mutex un pthread_cond, lai nodrošinātu drošu piekļuvi kopīgajiem resursiem un sinhronizāciju.

Klients arī uztur savienojumu izmantojot SO_KEEPALIVE, kā arī periodiskas "ping" ziņas, lai uzturētu savienojumu un noteiktu, vai serveris ir pieejams.

Serveris arī izmanto trīs pavedienus, kuri sazinas viens ar otru caur diviem kopīgiem atmiņas gabaliem.
"rx" pavediens (definēts rx.c/rx.h) ir atbildīgais par klausīšanos un klientu pieprasījumu gaidīšanu. Šis pavediens izmanto epoll, lai sekotu līdzi visiem klientiem, kuri ir izveidojuši savienojumu, un gaida, kad kāds klients kaut ko sūtīs. Kad kaut ko sūta, šis pavediens gaida, līdz ir saņemts viss, un pēc tam to ieraksta "input" MessageQueue mainīgajā, kas ir kopīgi pieejams "rx" un "game" pavedienam, izmantojot pthread aizslēgšanas iespējas, lai varētu droši rakstīt un lasīt.
"game" pavediens (definēts game.c/game.h) ir atbildīgais par pašu spēles loģiku. Pats par sevi serveris ik pa noteiktu laiku (~16 ms, bet iespējams viegli mainīt) palaiž "gametick", kas pārbauda, vai ir kādi spēlētāji iekāpuši sprāgstvielas, vai nav beigušies sprādzienu, utt. Papildus taktīm, ja ir kaut kas ierakstīts "input" MessageQueue, šis pavediens veic atbilstošās darbības (piemēram, ja ir atnākusi MOVE_ATTEMPT ziņa, tad šis pavediens mēģina pakustināt spēlētāju, un, ja tas ir izdevies (nav veikts aizliegts gājiens), padot tālāk to "tx" pavedienam, kurš tālāk par to informē klientus).
"tx" pavediens (definēts tx.c/rx.h) darbojas līdzīgi kā "rx" un "game". Ja "game" pavediens vēlas, lai kāda ziņa ir nosūtīta klientiem, tad viņš padod to ziņu caur "output" MessageQueue "tx" pavedienam, kurš tad to mēģina nosūtīt pašiem klientiem.

Šī sadale tika veidota ar domu, lai paša spēles loģikas ātrumu pēc iespējas mazāk ietekmētu svārstības nosūtīšana un saņemšanā, jo tām ir daudz neparedzamāks darbības ātrums, nekā parastam C kodam. Sanāk tā, ka, ja "game" pavedienam ir svarīgi nodot klientiem, ka kaut kas ir pakustējies, tā kā "game" pavedienam ir tā jau jāatbild par daudz ko citu (pašu spēles loģiku), viņs to nodot "tx" pavedienam, pēc kura brīzā tā vairs nav viņa problēma un var uzreiz atgriezties pie spēles loģikas un taktīm, kāmēr "tx" pavediens pats neatkarīgi mēģinās nosūtīt šo gājienu.
