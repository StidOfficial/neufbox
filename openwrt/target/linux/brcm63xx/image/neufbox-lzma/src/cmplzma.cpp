/*
<:copyright-gpl
 Copyright 2008 Broadcom Corp. All Rights Reserved.

 This program is free software; you can distribute it and/or modify it
 under the terms of the GNU General Public License (Version 2) as
 published by the Free Software Foundation.

 This program is distributed in the hope it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
:>
*/
/***************************************************************************
 * File Name  : cmplzma.c
 *
 * Description: This program compress binary file with lzma compression method
 *              It requires big endian mips32 ELF and the bin files as the input 
 *              for CFE and vmlinux.
 *
 *              For CFE RAM compress:
 *              Command: cmplzma [-t] -c -2 cfe cfe.bin flashimg.S
 *              where cfe is elf, cfe.bin is 
 *              binary file to be compressed and flashimg.S is  Asm data array for
 *              the compressed binary to be linked in as data. 
 *
 *              For vmlinux:
 *              Command: cmplzma [-t] -k -2 vmlinux vmlinux.bin vmlinux.lz
 *              where vmlinux is the elf file, vmliux.bin the binary file
 *              and vmlinux.lz is the compressed output
 *
 * Updates    : 04/08/2003  seanl.  Created.
 *
 ***************************************************************************/

/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <time.h>

#include "7z.h"


// elf structs and defines from CFE

/* p_type */
#define	PT_NULL		0		/* Program header table entry unused */
#define PT_LOAD		1		/* Loadable program segment */
#define PT_DYNAMIC	2		/* Dynamic linking information */
#define PT_INTERP	3		/* Program interpreter */
#define PT_NOTE		4		/* Auxiliary information */
#define PT_SHLIB	5		/* Reserved, unspecified semantics */
#define PT_PHDR		6		/* Entry for header table itself */
#define PT_LOPROC	0x70000000	/* Processor-specific */
#define PT_HIPROC	0x7FFFFFFF	/* Processor-specific */

#define CFE_ERR_NOTELF		-11
#define CFE_ERR_NOT32BIT 	-12
#define CFE_ERR_WRONGENDIAN -13
#define CFE_ERR_BADELFVERS 	-14
#define CFE_ERR_NOTMIPS 	-15
#define CFE_ERR_BADELFFMT 	-16
#define CFE_ERR_BADADDR 	-17

/* e_indent */
#define EI_MAG0		 0		/* File identification byte 0 index */
#define EI_MAG1		 1		/* File identification byte 1 index */
#define EI_MAG2		 2		/* File identification byte 2 index */
#define EI_MAG3		 3		/* File identification byte 3 index */
#define EI_CLASS	 4		/* File class */

#define ELFCLASSNONE 0		 /* Invalid class */
#define ELFCLASS32	 1		 /* 32-bit objects */
#define ELFCLASS64	 2		 /* 64-bit objects */
#define EI_DATA		 5		/* Data encoding */

#define ELFDATANONE	 0		 /* Invalid data encoding */
#define ELFDATA2LSB	 1		 /* 2's complement, little endian */
#define ELFDATA2MSB	 2		 /* 2's complement, big endian */
#define EI_VERSION	 6		/* File version */
#define EI_PAD		 7		/* Start of padding bytes */

#define ELFMAG0		0x7F	/* Magic number byte 0 */
#define ELFMAG1		'E'		/* Magic number byte 1 */
#define ELFMAG2		'L'		/* Magic number byte 2 */
#define ELFMAG3		'F'		/* Magic number byte 3 */

typedef unsigned short	Elf32_Half;
typedef unsigned int	Elf32_Word;
typedef signed int	    Elf32_Sword;
typedef unsigned int	Elf32_Off;
typedef unsigned int	Elf32_Addr;
typedef unsigned char	Elf_Char;
/*
 * ELF File Header 
 */
#define EI_NIDENT	16
typedef struct {
    Elf_Char	e_ident[EI_NIDENT];
    Elf32_Half	e_type;
    Elf32_Half	e_machine;
    Elf32_Word	e_version;
    Elf32_Addr	e_entry;
    Elf32_Off	e_phoff;
    Elf32_Off	e_shoff;
    Elf32_Word	e_flags;
    Elf32_Half	e_ehsize;
    Elf32_Half	e_phentsize;
    Elf32_Half	e_phnum;
    Elf32_Half	e_shentsize;
    Elf32_Half	e_shnum;
    Elf32_Half	e_shstrndx;
} Elf32_Ehdr;

/*
 * Program Header 
 */
typedef struct {
  Elf32_Word	p_type;			/* Identifies program segment type */
  Elf32_Off	    p_offset;		/* Segment file offset */
  Elf32_Addr	p_vaddr;		/* Segment virtual address */
  Elf32_Addr	p_paddr;		/* Segment physical address */
  Elf32_Word	p_filesz;		/* Segment size in file */
  Elf32_Word	p_memsz;		/* Segment size in memory */
  Elf32_Word	p_flags;		/* Segment flags */
  Elf32_Word	p_align;		/* Segment alignment, file & memory */
} Elf32_Phdr;

int getElfInfo(char *elfFile, Elf32_Addr *eEntry, Elf32_Addr *pVaddr);

// Check the elf file validity and extract the program entry and text 
// start address
int getElfInfo(char *elfFile, Elf32_Addr *eEntry, Elf32_Addr *pVaddr)
{
    Elf32_Ehdr *ep;
    Elf32_Phdr *phtab = 0;
    unsigned int nbytes;
    int i;
    Elf32_Ehdr ehdr;
    FILE *hInput;

    if ((hInput = fopen(elfFile, "rb")) == NULL)
    {
        printf("Error open file: %s\n", elfFile);
        return -1;
    }

    if (fread((char *) &ehdr, sizeof(char), sizeof(ehdr), hInput) != sizeof(ehdr))
    {
        printf("Error reading file: %s\n", elfFile);
        return -1;
	}

    ep = &ehdr;

    *eEntry = ep->e_entry;

    /* check header validity */
    if (ep->e_ident[EI_MAG0] != ELFMAG0 ||
        ep->e_ident[EI_MAG1] != ELFMAG1 ||
	    ep->e_ident[EI_MAG2] != ELFMAG2 ||
	    ep->e_ident[EI_MAG3] != ELFMAG3) 
    {
        printf("Not ELF file\n");
	    return CFE_ERR_NOTELF;
	}

    if (ep->e_ident[EI_CLASS] != ELFCLASS32) 
    {
        printf("Not 32 bit elf\n");
        return CFE_ERR_NOT32BIT;
    }
    
    if (ep->e_ident[EI_DATA] != ELFDATA2MSB) 
    {
        printf("Not BIG Endian\n");
        return CFE_ERR_WRONGENDIAN;	/* big endian */
    }

    /* Is there a program header? */
    if (ep->e_phoff == 0 || ep->e_phnum == 0)
    {
        printf("No program header? Wrong elf file\n");
	    return CFE_ERR_BADELFFMT;
	}

    /* Load program header */
    ep->e_phnum = htons(ep->e_phnum);
    ep->e_phoff = htonl(ep->e_phoff);
    nbytes = ep->e_phnum * sizeof(Elf32_Phdr);
    phtab = (Elf32_Phdr *) malloc(nbytes);
    if (!phtab) 
    {
	    printf("Failed to malloc memory!\n");
        return -1;
	}

    if (fseek(hInput, ep->e_phoff, SEEK_SET)!= 0)
    {
	    free(phtab);
        printf("File seek error\n");
	    return -1;
	}
    if (fread((unsigned char *)phtab, sizeof(char), nbytes, hInput) != nbytes)
    {
	    free(phtab);
        printf("File read error\n");
	    return -1;
	}

	for (i = 0; i < ep->e_phnum; i++)
    {
	    Elf32_Off lowest_offset = ~0;
	    Elf32_Phdr *ph = 0;
        ph = &phtab[i];
        phtab[i].p_offset = htonl(phtab[i].p_offset);
        phtab[i].p_type = htonl(phtab[i].p_type);
	    if ((phtab[i].p_type == PT_LOAD) && (phtab[i].p_offset < lowest_offset)) 
        {
	        ph = &phtab[i];
	        lowest_offset = ph->p_offset;
            *pVaddr = ph->p_vaddr;      // found the text start address
            return 0;
	    }
    }
    printf("No text start address found! Wrong elf file ?\n");
    return -1;
}

void usage(char *pName)
{
    printf("Example:\n");
    printf("To compress CFE ram     :  %s -c -2 inputElfFile inputBinFile outputAsmFile\n", pName);
    printf("To compress linux Kernel:  %s -k -2 inputElfFile inputBinFile outputCompressedFile\n\n", pName);
    printf("NOTE: -2 is the default compression level.  Allowable levels are -1 through -3\n");
    printf("where -3 may yield better compression ratio but slower. For faster compression, use -1\n");
    exit(-1);
}

/*************************************************************
 * Function Name: main
 * Description  : Program entry point that parses command line parameters
 *                and calls a function to create the image.
 * Returns      : 0
 ***************************************************************************/
int main (int argc, char **argv)
{
    FILE *hInput = NULL, *hOutput = NULL;
    struct stat StatBuf;
    char inputElfFile[256], inputBinFile[256], outputFile[256], segment[6];
    unsigned char *inData, *outData;
    bool status;
    unsigned long outLen, inLen;
    int cmpKernel = 0;
    Elf32_Addr entryPoint;
    Elf32_Addr textAddr;
    unsigned lzma_algo;
    unsigned lzma_dictsize;
    unsigned lzma_fastbytes;
    int lzma_compression_level; 
    int ret;
    char *pgmName = argv[0];

    if (argc == 7 && strcmp(argv[1], "-t") == 0)
    {
        strcpy(segment, ".text");
        argc--;
        argv++;
    }
    else
        strcpy(segment, ".data");

    if (argc != 6)
        usage(pgmName);

    if (strcmp(argv[1], "-k") == 0) 
        cmpKernel = 1;
    else if (strcmp(argv[1], "-c") != 0)
        usage(pgmName);
    
    if (strcmp(argv[2], "-1") == 0)
        lzma_compression_level = 1;
    else if (strcmp(argv[2], "-2") == 0)
        lzma_compression_level = 2;
    else if (strcmp(argv[2], "-3") == 0)
        lzma_compression_level = 3;
    else
        usage(pgmName);

    strcpy(inputElfFile, argv[3]);
    strcpy(inputBinFile, argv[4]);
    strcpy(outputFile, argv[5]);

    if ((ret = getElfInfo(inputElfFile, &entryPoint, &textAddr)) != 0)
        return -1;

    printf("Code text starts: textAddr=0x%08X  Program entry point: 0x%08X,\n", 
        (unsigned int)(htonl(textAddr)), (unsigned int)(htonl(entryPoint)));

    if (stat(inputBinFile, &StatBuf ) == 0 && (hInput = fopen(inputBinFile, "rb" )) == NULL)
    {
        printf( "Error opening input file %s.\n\n", inputBinFile);
        return -1;
    }

    inData = (unsigned char *) malloc(StatBuf.st_size);
    outData = (unsigned char *) malloc((StatBuf.st_size+2) * 5);        // make sure we have enough output buf

    if (!inData || !outData)
    {
        printf( "Memory allocation error, in=0x%8.8lx, out=0x%8.8lx.\n\n",
            (unsigned long) inData, (unsigned long) outData);
        fclose( hInput );
        return -1;
    }

    if (fread(inData, sizeof(char), StatBuf.st_size, hInput) != StatBuf.st_size)
    {
        printf( "Error read input file %s.\n\n", inputBinFile);
        return -1;
    }

    inLen = StatBuf.st_size;
    
    // compression function
    switch (lzma_compression_level)
    {
        case 1 :
            lzma_algo = 1;
            lzma_dictsize = 1 << 20;
            lzma_fastbytes = 64;
            break;
        case 2 :
            lzma_algo = 2;
            lzma_dictsize = 1 << 22;
            lzma_fastbytes = 128;
            break;
        case 3 :
            lzma_algo = 2;
            lzma_dictsize = 1 << 24;
            lzma_fastbytes = 255;
            break;
        default :
            printf("Invalid LZMA compression level.");
    }
    outLen = 23*1024*1024;
    status = compress_lzma_7z((const unsigned char*) inData, 
                               (unsigned) inLen, 
                               (unsigned char*) outData, 
                               (unsigned &) outLen, 
                               lzma_algo, 
                               lzma_dictsize,
                               lzma_fastbytes);
    if (status != true)
    {
        /* this should NEVER happen */
        printf("Internal error - LZMA compression failed.\n");
        return false;
    }
    
    /* Open output file. */
    if ((hOutput = fopen(outputFile, "w+" )) == NULL)
    {
        printf ("Error opening output file %s.\n\n", outputFile);
        return -1;
    }

    if (cmpKernel)
    {
        unsigned long swapedOutLen = htonl(outLen);    //little Endia on build host and big Endia on target
        if (fwrite(&textAddr, sizeof(Elf32_Addr), 1, hOutput) != 1 || 
            fwrite(&entryPoint, sizeof(Elf32_Addr), 1, hOutput) != 1 || 
            fwrite(&swapedOutLen, sizeof(unsigned long), 1, hOutput) != 1  ||      
            fwrite(outData, sizeof(char), outLen, hOutput) != outLen)
            printf( "Error writing to output file.\n\n" );
    }
    else // write the asm file for CFE
    {
        struct tm *newtime;
        time_t aclock;
        int i;
        unsigned char *curPtr = outData, *endPtr = outData + outLen;
        unsigned char *entryPtr = (unsigned char*) &entryPoint;

        time( &aclock );                 /* Get time in seconds */
        newtime = localtime( &aclock );  /* Convert time to struct */
        fprintf(hOutput, "/* Convert binary to asm\n   Input file : %s\n   Output file: %s\n   Date       : %s*/\n"
            , inputBinFile, outputFile, asctime(newtime));
        fprintf(hOutput, "%s", "\t.globl _binArrayStart\n");
        fprintf(hOutput, "%s", "\t.globl _binArrayEnd\n");
        fprintf(hOutput, "\t%s\n\n", segment);
        fprintf(hOutput, "%s", "_binArrayStart:");
        // write 4 bytes of entry point first
        fprintf(hOutput, "%s%04o", "\n\t\t.byte ", (unsigned int) *entryPtr++);
        fprintf(hOutput, ",%04o", (unsigned int) *entryPtr++);
        fprintf(hOutput, ",%04o", (unsigned int) *entryPtr++);
        fprintf(hOutput, ",%04o", (unsigned int) *entryPtr);
        i = 4;
        for (; curPtr < endPtr; curPtr++)
        {   
            if (i == 0)
                fprintf(hOutput, "%s%04o", "\n\t\t.byte ", (unsigned int) *curPtr);
            else
                fprintf(hOutput, ",%04o", (unsigned int) *curPtr);
            if (++i == 16)
                i = 0;
        }
        fprintf(hOutput, "%s", "\n_binArrayEnd:\n");
    }

    fclose( hOutput );

    printf("Before compression: %ld  After compression (level=%d): %ld\n\r", inLen, lzma_compression_level, outLen);
    printf("Percent Compression = %.2f\n\r", (float)((float)(inLen - outLen)/(float)inLen)*(float)100);

    if(inData)
        free(inData);
    if(outData)
        free(outData);
    fclose(hInput);

    return(0);
}


