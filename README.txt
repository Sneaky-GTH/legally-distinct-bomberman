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

Klients arī uztur savienojumu izmantojot SO_KEEPALIVE, kā arī periodiskas "ping" ziņas, lai uzturētu savienojumu un noteiktu, vai serveris ir pieejams.

TODO: servera docs
