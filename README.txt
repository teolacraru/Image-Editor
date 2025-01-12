// 311CA Lacraru Teodora-Ioana

	Editor de imagini

		Se considera implementarea unui editor de text ce poate aplica anumite
	operatii aspura unor imagini formatate PPM sau PGM. Codul este scris in C
	si suporta operatii precum P2, P3, P5, P6.

		"image" este structura de baza pentru o imagine si retine diferite
	caracteristici a ceasteia, precum latimea, inaltimea, valoarea maxima a
	pixelilor, formatul, datele imaginii (pixelii) si anumite informatii in
	legatura cu starea imaginii ce vor ajuta la urmatorele operatii.
		Functia "free_img" elibereaza memoria alocata datelor unei imagini,
	"isloaded" verifica daca a fost deja incarcata o imagine.
		"p2_p5_memalloc" si "p3_p6_memalloc" aloca memorie imaginilor P2, P5,
	respectiv P3, P6, care contin pixeli color. "clampp" incadreaza o valoare
	intre 0 si 255, functie pe care o folosesc la anumite operatii. "load"
	incarca o imagine in fisier si gestioneaza citirea datelor.
		"select_coord" si "select_all" sunt functiile care permit selectarea
	unei anumite regiuni din imagine, respectiv intreaga imagine.
		"crop" decupeaza imaginea pe dimensiunile selectate de functiile
	precedente.
		"histogram" calculeaza si afiseaza histograma unei imagini in tonuri
	de gri.
		"equalize" egalizeaza histogramele, "rotate" roteste imaginea spre
	dreapta sau spre stanga la un unghi specificat. "apply" permite
	aplicarea unui filtru(kernel) asupra imaginei, ajutata de functia
	"matrixmultiplication".
	
		In final, in functia main identific comenzile pentru operatiile
	respective si aplic functiile corespunzatoare.
