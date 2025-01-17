/*MusyX (GBC) to MIDI converter*/
/*By Will Trowbridge*/
/*Portions based on code by ValleyBell*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define bankSize 16384

FILE* rom, * txt, * raw;
long bank;
long offset;
long tablePtrLoc;
long tableOffset;
int i, j;
char outfile[1000000];
int songNum;
int numSongs;
int sampNum;
int numSamp;
int mode = 1;
long bankAmt;
int foundTable = 0;
int curMask = 0;
int masterBank = 0;

long asdrTab;
long sfxTab;
int numSfx;
long samTab;
int sampMap;
long numSampMap;
long songTab;
int curInst = 0;
long sampPtr = 0;
long sampSize = 0;
int sampQ = 0;

char* argv3;

unsigned static char* romData;
unsigned static char* exRomData;

const char MagicBytes[6] = { 0xAF, 0xC9, 0xFC, 0xF3, 0xCF, 0x3F };

/*Function prototypes*/
unsigned short ReadLE16(unsigned char* Data);
unsigned short ReadBE16(unsigned char* Data);
static void Write8B(unsigned char* buffer, unsigned int value);
static void WriteBE32(unsigned char* buffer, unsigned long value);
static void WriteBE24(unsigned char* buffer, unsigned long value);
static void WriteBE16(unsigned char* buffer, unsigned int value);
void song2txt(int songNum, int bank, long ptr);
void sam2raw(int sampNum, long ptr, long size, int bank, int quality);

/*Convert little-endian pointer to big-endian*/
unsigned short ReadLE16(unsigned char* Data)
{
	return (Data[0] << 0) | (Data[1] << 8);
}

/*Store big-endian pointer*/
unsigned short ReadBE16(unsigned char* Data)
{
	return (Data[0] << 8) | (Data[1] << 0);
}

static void Write8B(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = value;
}

static void WriteBE32(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF000000) >> 24;
	buffer[0x01] = (value & 0x00FF0000) >> 16;
	buffer[0x02] = (value & 0x0000FF00) >> 8;
	buffer[0x03] = (value & 0x000000FF) >> 0;

	return;
}

static void WriteBE24(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF0000) >> 16;
	buffer[0x01] = (value & 0x00FF00) >> 8;
	buffer[0x02] = (value & 0x0000FF) >> 0;

	return;
}

static void WriteBE16(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = (value & 0xFF00) >> 8;
	buffer[0x01] = (value & 0x00FF) >> 0;

	return;
}

int main(int args, char* argv[])
{
	printf("MusyX (GBC) to TXT converter\n");
	if (args < 3)
	{
		printf("Usage: MX2TXT <rom> <bank> <parameter>\n");
		printf("Parameters: M = extract music (default), S = extract samples\n");
		return -1;
	}
	else
	{
		if (args == 3)
		{
			mode = 1;
		}
		else {
			argv3 = argv[3];

			/*Extract music to MIDI*/
			if (strcmp(argv3, "m") == 0 || strcmp(argv3, "M") == 0)
			{
				mode = 1;
			}

			/*Extract samples*/
			else if (strcmp(argv3, "s") == 0 || strcmp(argv3, "S") == 0)
			{
				mode = 2;
			}
			else
			{
				printf("ERROR: Invalid mode switch!\n");
				exit(1);
			}
		}
		if ((rom = fopen(argv[1], "rb")) == NULL)
		{
			printf("ERROR: Unable to open file %s!\n", argv[1]);
			exit(1);
		}
		else
		{
			masterBank = strtol(argv[2], NULL, 16);
			bankAmt = bankSize;

			fseek(rom, ((masterBank - 1) * bankSize), SEEK_SET);
			romData = (unsigned char*)malloc(bankSize);
			fread(romData, 1, bankSize, rom);

			/*Try to search the bank for song table loader - Method 1: Mega Man 3/Bionic Commando*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytes, 6)) && foundTable != 1)
				{
					tableOffset = i + 6 + bankAmt;
					printf("Found project data at address 0x%04x!\n", tableOffset);
					foundTable = 1;
				}
			}

			if (foundTable == 1)
			{

				/*Get the MusyX project data parameters*/
				i = tableOffset - bankAmt;
				asdrTab = ReadLE16(&romData[i]) + tableOffset;
				printf("ASDR table address: 0x%04X\n", asdrTab);
				i += 2;
				sfxTab = ReadLE16(&romData[i]) + tableOffset;
				printf("Sound effect table address: 0x%04X\n", sfxTab);
				i += 2;
				numSfx = romData[i];
				printf("Number of sound effects: %i\n", numSfx);
				i++;
				samTab = ReadLE16(&romData[i]) + tableOffset;
				printf("Sample table address: 0x%04X\n", samTab);
				i += 2;
				numSamp = romData[i];
				printf("Number of samples: %i\n", numSamp);
				/*Additionally skip an "empty" 1-byte parameter*/
				i += 2;
				sampMap = ReadLE16(&romData[i]) + tableOffset;
				printf("Sample map address: 0x%04X\n", sampMap);
				i += 2;
				printf("Number of entires in sample map: %i\n", numSampMap);
				i++;
				songTab = ReadLE16(&romData[i]) + tableOffset;
				printf("Song table address: 0x%04X\n", songTab);
				i += 2;
				numSongs = romData[i];
				printf("Number of songs: %i\n", numSongs);

				if (mode == 1)
				{
					/*Now convert all the songs*/
					i = songTab - bankAmt;
					for (songNum = 1; songNum <= numSongs; songNum++)
					{
						bank = romData[i] + masterBank;
						offset = ReadLE16(&romData[i + 1]);
						printf("Song %i: bank %01X, address 0x%04X\n", songNum, bank, offset);
						song2txt(songNum, bank, offset);
						i += 3;
					}
				}

				else if (mode == 2)
				{
					/*Extract all the samples*/
					i = samTab - bankAmt;
					for (sampNum = 1; sampNum <= numSamp; sampNum++)
					{
						sampPtr = ReadLE16(&romData[i]);
						sampSize = ReadLE16(&romData[i + 2]) * 0x10;
						sampQ = romData[i + 4];
						bank = romData[i + 5] + masterBank;
						printf("Sample %i: offset: 0x%04X, size: %01X, quality: %01X, bank: %01X\n", sampNum, sampPtr, sampSize, sampQ, bank);
						sam2raw(sampNum, sampPtr, sampSize, bank, sampQ);
						i += 6;

					}
				}


				printf("The operation was succesfully completed!\n");
				exit(0);
			}
			else
			{
				printf("ERROR: Magic bytes not found!\n");
				exit(-1);
			}

		}
	}
}

void song2txt(int songNum, int bank, long ptr)
{
	int seqPos = 0;
	int maskArray[16];
	int numPat = 0;
	int numTracks = 0;
	int curTrack = 0;
	int seqEnd = 0;
	int curNote = 0;
	int curNoteLen = 0;
	long curDelay = 0;
	int curVel = 0;
	int tempo = 0;
	int effect = 0;
	int k = 0;
	long trackTab = 0;
	long patTab = 0;
	long patterns[4];
	long tracks[8];
	int chanStart[4][2];
	unsigned char mask = 0;
	int activeChan[4] = { 0, 0, 0, 0 };
	unsigned char command[4];
	int curPat = 0;
	long patBase = 0;
	long endPat = 0;
	int progC = 0;
	int noteL = 0;
	unsigned char lowNibble = 0;
	unsigned char highNibble = 0;

	sprintf(outfile, "song%i.txt", songNum);
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file song%i.txt!\n", songNum);
		exit(2);
	}
	else
	{
		/*Copy the song data's bank*/
		fseek(rom, ((bank - 1) * bankSize), SEEK_SET);
		exRomData = (unsigned char*)malloc(bankSize);
		fread(exRomData, 1, bankSize, rom);

		/*Go to the song's position*/
		seqPos = ptr - bankAmt;

		/*Get header information*/
		command[0] = exRomData[seqPos];
		command[1] = exRomData[seqPos + 1];
		command[2] = exRomData[seqPos + 2];
		command[3] = exRomData[seqPos + 3];

		fprintf(txt, "Default program mapping:\n");
		for (curTrack == 0; curTrack < 4; curTrack++)
		{
			fprintf(txt, "Channel %i: %i\n", (curTrack + 1), command[curTrack]);
		}

		fprintf(txt, "Channel mapping: ");
		seqPos += 4;
		for (k = 0; k < 128; k++)
		{
			command[0] = exRomData[seqPos];
			fprintf(txt, "%i", command[0]);
			if (k != 127)
			{
				fprintf(txt, ", ");
			}
			seqPos++;
		}
		fprintf(txt, "\n");
		patBase = seqPos;
		trackTab = ReadBE16(&exRomData[seqPos]);
		fprintf(txt, "Track table: 0x%01X\n", trackTab);
		seqPos += 2;
		patTab = ReadBE16(&exRomData[seqPos]);
		fprintf(txt, "Pattern table: 0x%01X\n", patTab);
		seqPos += 2;
		tempo = ReadBE16(&exRomData[seqPos]);
		fprintf(txt, "Tempo: %i\n\n", tempo);
		seqPos += 2;

		/*Get the track pattern pointers*/
		for (curTrack = 0; curTrack < 4; curTrack++)
		{
			patterns[curTrack] = ReadBE16(&exRomData[seqPos]);
			fprintf(txt, "Channel %i pattern: 0x%04X\n", (curTrack + 1), patterns[curTrack]);
			seqPos += 2;
		}
		fprintf(txt, "\n");

		/*Get the track pattern data*/
		for (curTrack = 0; curTrack < 4; curTrack++)
		{
			if (patterns[curTrack] != 0x0000)
			{
				numTracks++;
				activeChan[curTrack]++;
				fprintf(txt, "Channel %i:\n", (curTrack + 1));
				seqPos = patBase + patterns[curTrack];
				curDelay = ReadBE16(&exRomData[seqPos]);
				seqPos += 2;
				curPat = exRomData[seqPos];
				fprintf(txt, "Pattern: %01X, start time: %01X\n", curPat, curDelay);
				seqPos++;
				if (exRomData[seqPos + 2] < 0xFE)
				{
					numTracks++;
					curDelay = ReadBE16(&exRomData[seqPos]);
					seqPos += 2;
					curPat = exRomData[seqPos];
					fprintf(txt, "Pattern: %01X, time to first event: %01X\n", curPat, curDelay);
					seqPos++;
					curDelay = ReadBE16(&exRomData[seqPos]);
					seqPos += 2;
				}

				if (exRomData[seqPos + 2] == 0xFE)
				{
					fprintf(txt, "Loop: time: %01X\n", curDelay);
					seqPos += 3;
					endPat = ReadBE16(&exRomData[seqPos]);
					curPat = exRomData[seqPos + 2];
					fprintf(txt, "Last pattern: %i, offset: 0x%04X", curPat, endPat);
				}
				else
				{
					fprintf(txt, "No loop\n");
				}
				
				fprintf(txt, "\n");
			}
		}

		fprintf(txt, "\n");
		seqPos = patTab + patBase;
		for (curTrack = 0; curTrack < numTracks; curTrack++)
		{
			tracks[curTrack] = ReadBE16(&exRomData[seqPos]);
			seqPos += 2;
			fprintf(txt, "Track %i: 0x%02X\n", (curTrack + 1), tracks[curTrack]);
		}
		fprintf(txt, "\n");

		for (curTrack = 0; curTrack < numTracks; curTrack++)
		{
				fprintf(txt, "Pattern %i:\n", (curTrack + 1));
				curInst = 0;
				seqPos = tracks[curTrack] + patBase;
				seqEnd = 0;

				while (seqEnd == 0)
				{
					command[0] = exRomData[seqPos];
					command[1] = exRomData[seqPos + 1];
					command[2] = exRomData[seqPos + 2];
					command[3] = exRomData[seqPos + 3];

					/*Rest*/
					if (command[0] == 0x00)
					{
						curDelay = command[1];
						seqPos += 2;

						/*Is there a program change?*/

						command[0] = exRomData[seqPos];
						progC = command[0] & 0x80;

						if (progC == 0x80)
						{
							curInst = command[0] & 0x7F;
							fprintf(txt, "Program change: %i\n", curInst);
							seqPos++;
						}
						fprintf(txt, "Rest, delay: %01X\n", curDelay);

					}

					/*End of track*/
					else if (command[0] == 0xF0 && command[1] == 0x00 && command[2] == 0xFF)
					{
						fprintf(txt, "End of track\n\n");
						seqEnd = 1;

					}

					/*Play note*/
					else
					{
						progC = 0;
						noteL = 0;

						/*Get the velocity/volume of the note*/
						curVel = command[0] & 0xF0;

						/*Get the note's timing/delay*/
						curDelay = command[1] + ((command[0] & 0x0F) * 0x100);

						seqPos += 2;

						command[0] = exRomData[seqPos];
						/*Get the current note*/
						progC = command[0] & 0x80;
						curNote = command[0] & 0x7F;

						/*Is there a program change?*/
						if (progC == 0x80)
						{
							seqPos++;
							command[0] = exRomData[seqPos];
							curInst = command[0];
							fprintf(txt, "Program change: %i\n", curInst);
						}

						seqPos++;
						command[0] = exRomData[seqPos];
						command[1] = exRomData[seqPos + 1];
						/*Get the current note's length*/
						noteL = command[0] & 0x80;

						if (noteL == 0x80)
						{
							/*Go to loop pattern*/
							if (command[0] == 0xFE)
							{
								curNoteLen = command[1] & 0x7F;
								seqPos++;
							}
							else
							{
								curNoteLen = command[1] + ((command[0] & 0x15) * 0x100);
								seqPos++;
							}

							fprintf(txt, "Note: %i, delay: %i, length: %i, velocity: %i\n", curNote, curDelay, curNoteLen, curVel);
							seqPos++;
						}
						else
						{
							curNoteLen = command[0];
							fprintf(txt, "Note: %i, delay: %i, length: %i, velocity: %i\n", curNote, curDelay, curNoteLen, curVel);
							seqPos++;
						}
					}
				}
		}
	}

}

void sam2raw(int sampNum, long ptr, long size, int bank, int quality)
{
	int sampPos = 0;
	int k = 0;
	int c = 0;

	sprintf(outfile, "sample%i.raw", sampNum);
	if ((raw = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file sample%i.raw!\n", sampNum);
		exit(2);
	}
	else
	{
		/*Copy the sample data's bank (x 2)*/
		fseek(rom, ((bank - 1) * bankSize), SEEK_SET);
		exRomData = (unsigned char*)malloc(bankSize * 2);
		fread(exRomData, 1, bankSize, rom);


		/*Go to the sample's position*/
		sampPos = ptr - bankAmt;

		for (k = 0; k < size; k++)
		{
			c = exRomData[sampPos];
			fputc(c, raw);
			sampPos++;
		}
		fclose(raw);
	}
}