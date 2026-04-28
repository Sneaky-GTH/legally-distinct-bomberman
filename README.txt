"Legally Distinct Bomberman"

Autori:
 - Kārlis Čerņavskis, kc24020 (projektā ieguldīja 50%)
 - Gustavs Jānis Ozoliņš, go23005 (projektā ieguldīja 50%)

Pirmkods arī pieejams GitHub repozitorijā: https://github.com/Sneaky-GTH/legally-distinct-bomberman

Projekta pirmkods ir zem src direktorijas. Tas ir sadalīts apakšdirektorijās sekojoši:
 - src/client: Klients, kas tiek kompilēts kā ./build/client
 - src/server: Serveris, kas tiek kompilēts kā ./build/server
 - src/lib: Kopīgā bibliotēka, kas tiek izmantots gan serverī, gan klientā. Tiek kompilēts kā ./build/libbomberman.so

Kopīgā bibliotēka tiek dinamiski saistīta (to neiekļauj kompilētajā serverī un klientā).
Centāmies atšifrēt projekta protokolu un pie tā pieturēties.

TODO: add more description
