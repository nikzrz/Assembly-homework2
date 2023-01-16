#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include "image_processing.cpp"

using namespace std;

void Dilation(int n, int filter_size, short* resimadres_org);
void Erosion(int n, int filter_size, short* resimadres_org);

int main(void) {
    int M, N, Q, i, j, filter_size;
    bool type;
    int efile;
    char org_resim[100], dil_resim[] = "dilated.pgm", ero_resim[] = "eroded.pgm";
    do {
        printf("Orijinal resmin yolunu (path) giriniz:\n-> ");
        scanf("%s", &org_resim);
        system("CLS");
        efile = readImageHeader(org_resim, N, M, Q, type);
    } while (efile > 1);
    int** resim_org = resimOku(org_resim);

    printf("Orjinal Resim Yolu: \t\t\t%s\n", org_resim);

    short *resimdizi_org = (short*) malloc(N*M * sizeof(short));

    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            resimdizi_org[i*N + j] = (short)resim_org[i][j];

    int menu;
    printf("Yapmak istediginiz islemi giriniz...\n");
    printf("1-) Dilation\n");
    printf("2-) Erosion\n");
    printf("3-) Cikis\n> ");
    scanf("%d", &menu);
    printf("Filtre boyutunu giriniz: ");
    scanf("%d", &filter_size);

    switch (menu){
        case 1:
            Dilation(N*M, filter_size, resimdizi_org);
            resimYaz(dil_resim, resimdizi_org, N, M, Q);
            break;
        case 2:
            Erosion(N*M, filter_size, resimdizi_org);
            resimYaz(ero_resim, resimdizi_org, N, M, Q);
            break;
        case 3:
            system("EXIT");
            break;
        default:
            system("EXIT");
            break;
    }

    system("PAUSE");
    return 0;
}

void Dilation(int n, int filter_size, short* resim_org) {
    __asm {
           //ECX, n değişkenini yükle ve ECX register'ına kopyala
            MOV ECX, n
            PUSH ECX
          //filter_size değişkenini ikiye katla (shift left) ve EDX register'ına yükle
            MOV EDX, filter_size
            SHL EDX, 0001h
          //ECX register'ına 0x200 (512) yükle
            MOV ECX, 0200h
          //filter_size değişkenini tekrar yükle
            MOV filter_size, EDX
          //EDX register'ını tekrar yarıya böl (shift right)
            SHR EDX, 0001h
          //ECX register'ını EDX register'ı ile çarp ve EAX register'ına yükle
            MOV EAX, ECX
            MUL EDX
          //EAX register'ını EDX register'ına kopyala
            MOV EDX, EAX
          //ECX register'ını geri yükle
            POP ECX
          //EDX register'ını iki katına çıkar
            ADD EDX, EAX
          //resim_org değişkenini EBX register'ına yükle
            MOV EBX, resim_org
          //; ECX register'ını 512'den çıkar
            SUB ECX, EAX
        
           // ESI register'ını 0 yap

    DNG1:   AND ESI, 0000h
          //EAX register'ını 255 yap
            MOV EAX, 00FFh
    DNG4:   XOR EDI, EDI
          //AX register'ındaki değeri EBX + ESI'deki değer ile karşılaştır. Eğer AX < WORD PTR[EBX + ESI] ise, AX'i WORD PTR[EBX + ESI] ile değiştir
            CMP AX, WORD PTR[EBX + ESI]
            JNA DNG3
            MOV AX, WORD PTR[EBX + ESI]
    DNG3:   ADD EDI, 0002h
          //ESI'yi EDI ile topla
            ADD ESI, EDI
         //AX register'ındaki değeri EBX + ESI'deki değer ile karşılaştır. Eğer AX < WORD PTR[EBX + ESI] ise, AX'i WORD PTR[EBX + ESI] ile değiştir
            CMP AX, WORD PTR[EBX + ESI]
            JNA DNG2
            MOV AX, WORD PTR[EBX + ESI]
    DNG2:   SUB ESI, EDI
            
            //EDI'yi filter_size ile karşılaştır, eğer EDI < filter_size ise, geri dön DNG3'e
            CMP EDI, [filter_size]
            JNG DNG3
            //; ESI'yi 1024 ile topla
            ADD ESI, 1024
            //ESI'yi EDX ile karşılaştır, eğer ESI < EDX ise, geri dön DNG4'e
            CMP ESI, EDX
            JNG DNG4
            //AX register'ındaki değeri EBX register'ındaki değere kopyala
            MOV WORD PTR[EBX], AX
           //EBX register'ını 2 ile topla
            ADD EBX, 0002h
          //DNG1'e geri dön
            LOOP DNG1
    }
    printf("\nDilation islemi sonucunda resim \"dilated.pgm\" ismiyle olusturuldu...\n");
}

void Erosion(int n, int filter_size, short* resim_org) {
    __asm {
            MOV ECX, n
            PUSH ECX
            MOV EDX, filter_size
            SHL EDX, 0001h
            MOV ECX, 0200h
            MOV filter_size, EDX
            SHR EDX, 0001h
            MOV EAX, ECX
            MUL EDX
            MOV EDX, EAX
            POP ECX
            ADD EDX, EAX
            MOV EBX, resim_org
            SUB ECX, EAX

    DNG1:   AND ESI, 0000h
            //yukarida delation ile farki 0000h girmesi EAX registerine
            MOV EAX, 0000h
    DNG4:   XOR EDI, EDI
            CMP AX, WORD PTR[EBX + ESI]
            JAE DNG3
            MOV AX, WORD PTR[EBX + ESI]
    DNG3:   ADD EDI, 0002h
            ADD ESI, EDI
            CMP AX, WORD PTR[EBX + ESI]
            JAE DNG2
            MOV AX, WORD PTR[EBX + ESI]
    DNG2:    SUB ESI, EDI
            CMP EDI, [filter_size]
            JNG DNG3
            ADD ESI, 1024
            CMP ESI, EDX
            JNG DNG4
            MOV WORD PTR[EBX], AX
            ADD EBX, 0002h
            LOOP DNG1
    }
    printf("\nErosion islemi sonucunda resim \"eroded.pgm\" ismiyle olusturuldu...\n");
}
