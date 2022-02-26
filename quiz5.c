/*****************************************************************************/
/*                 TESTER/PARSER: DO NOT CHANGE ANYTHING HERE                */
/*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define MaxBufferSize 200

/**
 * Each memory segment (block) is represented by a memorySegment structure object.
 */
typedef struct memorySegment {
    uint16_t startAddress;
    uint16_t length;
    bool occupied;
    struct memorySegment *next;
} memorySegment;

int newLen = 0;
int newAddress = 0;

/**
 * Functions for the actual handling of the memory segments. You need to implement these!
 */
void printList(memorySegment *memList);

void insertListItemAfter(memorySegment *current);

void removeListItemAfter(memorySegment *current);

memorySegment *initializeMemory();

memorySegment *assignFirst(memorySegment *memList, uint16_t requestedMem);

memorySegment *assignBest(memorySegment *memList, uint16_t requestedMem);

memorySegment *assignNext(memorySegment *memList, uint16_t requestedMem);

void reclaim(memorySegment *memList, memorySegment *thisOne);

memorySegment *assignFirstDyn(memorySegment *memList, uint16_t requestedMem);

memorySegment *assignBestDyn(memorySegment *memList, uint16_t requestedMem);

memorySegment *assignNextDyn(memorySegment *memList, uint16_t requestedMem);

void reclaimDyn(memorySegment *memList, memorySegment *thisOne);


void parseMessage(char *buffer, size_t size);

void execute(char *token, memorySegment *(*assignMemory)(memorySegment *mem, uint16_t size),
             void (*reclaimMemory)(memorySegment *mem, memorySegment *thisOne),
             memorySegment *memList, char *savePointer1, char *savePointer2);

memorySegment *initializeStaticMemory(int memorySize, int blockSize);

memorySegment *initializeDynamicMemory(int memorySize);

int main() {
    char buff[MaxBufferSize];
    parseMessage(buff, sizeof(buff));
}

void parseMessage(char *buffer, size_t size) {
    /* read the string from the stdin and check for error */
    if (fgets(buffer, size, stdin) == NULL) {
        printf("Error reading stdint.");
        exit(1);
    }

    /* pointer to the memory, which is represented by a linked list */
    memorySegment *memList;

    /* declare the delimiter, so that the string is divided in separate tokens */
    const char delimiter[2] = " ";

    /* used to specify on which string, the strtok_r performs */
    char *savePointer1 = NULL;
    char *savePointer2 = NULL;
    char *savePointer3 = NULL;
    char *savePointer4 = NULL;

    /* reading of the string's header with the necessary information about the test */
    char *sizeOfMemory = strtok_r(buffer, delimiter, &savePointer1);
    char *typeOfMemory = strtok_r(NULL, delimiter, &savePointer1);
    char *assignMethod = strtok_r(NULL, delimiter, &savePointer1);

    /* array of pointers to the appropriate memory management methods */
    memorySegment *(*methodOfAssignement)(memorySegment *memList, uint16_t requestedMem);
    void (*methodOfReclaim)(memorySegment *memList, memorySegment *thisOne);

    if (typeOfMemory[0] == 'S') {
        if (strcmp(assignMethod, "AF") == 0) {
            methodOfAssignement = assignFirst;
        } else if (strcmp(assignMethod, "AB") == 0) {
            methodOfAssignement = assignBest;
        } else if (strcmp(assignMethod, "AN") == 0) {
            methodOfAssignement = assignNext;
        } else {
            printf("Unknown memory assignement method.");
            exit(1);
        }
        methodOfReclaim = reclaim;
        char *blockSize = strtok_r(typeOfMemory, "S", &savePointer2);
        memList = initializeStaticMemory(atoi(sizeOfMemory), atoi(blockSize));
    } else if (typeOfMemory[0] == 'D') {
        if (strcmp(assignMethod, "AF") == 0) {
            methodOfAssignement = assignFirstDyn;
        } else if (strcmp(assignMethod, "AB") == 0) {
            methodOfAssignement = assignBestDyn;
        } else if (strcmp(assignMethod, "AN") == 0) {
            methodOfAssignement = assignNextDyn;
        } else {
            printf("Unknown memory assignement method.");
            exit(1);
        }
        methodOfReclaim = reclaimDyn;
        memList = initializeDynamicMemory(atoi(sizeOfMemory));
    } else {
        printf("Invalid memory type.");
        exit(1);
    }

    char *token = strtok_r(NULL, delimiter, &savePointer1);

    while (true) {
        token = strtok_r(NULL, delimiter, &savePointer1);
        if (token == NULL) {
            break;
        }
        execute(token, (*methodOfAssignement), methodOfReclaim, memList, savePointer3, savePointer4);
    }
    printList(memList);

}

void execute(char *token, memorySegment *(*assignMemory)(memorySegment *mem, uint16_t size),
             void (*reclaimMemory)(memorySegment *mem, memorySegment *thisOne),
             memorySegment *memList, char *savePointer1, char *savePointer2) {
    if (token[0] == 'A') {
        char *requestedMemory = strtok_r(token, "A", &savePointer1);
        (*assignMemory)(memList, atoi(requestedMemory));
    } else if (token[0] == 'R') {
        int indexOfBlockToReclaim = atoi(strtok_r(token, "R", &savePointer2));
        memorySegment *blockToReclaim = memList;
        if (indexOfBlockToReclaim <= 0) {   // 1-based, first block is block-1
            exit(1);
        }
        if (indexOfBlockToReclaim > 1) {
            while (indexOfBlockToReclaim > 1) {
                blockToReclaim = blockToReclaim->next;
                indexOfBlockToReclaim--;
            }
        }
        (*reclaimMemory)(memList, blockToReclaim);
    }
}


memorySegment *initializeStaticMemory(int memorySize, int blockSize) {
    int numberOfBlocks = memorySize / blockSize;
    int remainderSize = memorySize % blockSize;

    memorySegment *firstBlock = (memorySegment *) malloc(sizeof(memorySegment));
    firstBlock->occupied = false;
    firstBlock->length = blockSize;
    firstBlock->startAddress = 0;

    memorySegment *previousSegment = firstBlock;

    for (int i = 0; i < numberOfBlocks - 1; i++) {
        memorySegment *nextMemorySegment = (memorySegment *) malloc(sizeof(memorySegment));
        nextMemorySegment->occupied = false;
        nextMemorySegment->startAddress = previousSegment->startAddress + blockSize;
        nextMemorySegment->length = blockSize;
        previousSegment->next = nextMemorySegment;
        previousSegment = nextMemorySegment;
    }
    if (remainderSize > 0) {
        memorySegment *lastMemorySegment = (memorySegment *) malloc(sizeof(memorySegment));
        lastMemorySegment->length = remainderSize;
        lastMemorySegment->occupied = false;
        lastMemorySegment->startAddress = previousSegment->startAddress + blockSize;
        previousSegment->next = lastMemorySegment;
    }
    return firstBlock;
}

memorySegment *initializeDynamicMemory(int memorySize) {
    memorySegment *memory = (memorySegment *) malloc(sizeof(memorySegment));
    memory->startAddress = 0;
    memory->length = memorySize;
    memory->occupied = false;
    return memory;
}


/*****************************************************************************/
/*           IMPLEMENTATIONS: YOU NEED TO IMPLEMENT THESE FUNCTIONS          */
/*****************************************************************************/


void printList(memorySegment *memList) {
    memorySegment *current = memList;

    do{
    printf("%d %d %s\n", current->startAddress, current->length,
         current->occupied ? "Occupied!" : "Free");
    current = current->next;
  }while(current != NULL);
}

void insertListItemAfter(memorySegment *current) {
    memorySegment *newItem;
    newItem = (memorySegment *) malloc(sizeof(memorySegment));
    newItem->length = newLen;
    newItem->startAddress = newAddress;
    newItem->occupied = false;
    // your code here to allocate new record newitem

    if (current) { // current !== NULL
        if (current->next) {
            newItem->next = current->next;
            current->next = newItem;
        } else {
            current->next = newItem;
        }
    } else {
        current = newItem;
    }
}

void removeListItemAfter(memorySegment *current) {
    if (current) {
        if (current->next) {
            if (current->next->next) {
                current->next = current->next->next;
            } else {
                current->next = NULL;
            }
        }
    }
}

memorySegment *assignFirst(memorySegment *memList, uint16_t requestedMem) {
    memorySegment *current = memList;
    while (current) {
        if (current->occupied) {
            current = current->next;
            continue;
        }
        if (!current->occupied) {
            if (current->length >= requestedMem) {
                current->occupied = true;
                return current;
            }
        }
        current = current->next;
    }
    return NULL;
}

memorySegment *assignBest(memorySegment *memList, uint16_t requestedMem) {
    memorySegment *current = memList;
    memorySegment *best;

    while (current) {
        if (!current->occupied) {
            if (current->length >= requestedMem) {
                if (!best) {
                    best = current;
                } else if (best->length > current->length) {
                    best = current;
                }
            }
        }
        current = current->next;
    }

    if (best) {
        best->occupied = true;
        best->length -= requestedMem;
        return best;
        // REDUCE SEG SIZE
    }
    return NULL;
}

memorySegment *LastBlock;

memorySegment *assignNext(memorySegment *memList, uint16_t requestedMem) {
    memorySegment *current;
    if (!LastBlock) {
        current = memList;
    } else {
        current = LastBlock;
    }
    while (current) {
        if (!current->occupied) {
            if (current->length >= requestedMem) {
                current->occupied = true;
                return current;
            }
        }
        current = current->next;
    }
    return NULL;
}

void reclaim(memorySegment *memList, memorySegment *thisOne) {
    memorySegment *current = memList;
    while (current) {
        if (current->startAddress == thisOne->startAddress) {
            current->occupied = false;
            break;
        }
        current = current->next;
    }
}

memorySegment *assignFirstDyn(memorySegment *memList, uint16_t requestedMem) {
    memorySegment *current = memList;
    while (current) {
        if (!current->occupied) {
            if (current->length == requestedMem) {
                current->occupied = true;
                return current;
            }
            if (current->length > requestedMem) {
                int memoryLeft = current->length - requestedMem;
                current->occupied = true;
                current->length = requestedMem;
                if (current->next) {
                    if (!current->next->occupied) {
                        current->next->startAddress = current->startAddress + requestedMem;
                        current->next->length += memoryLeft;
                        return current;
                    }
                }
                newLen = memoryLeft;
                newAddress = current->startAddress + requestedMem;
                insertListItemAfter(current);
                return current;
            }
        }
        current = current->next;
    }
    return NULL;
}

memorySegment *assignBestDyn(memorySegment *memList, uint16_t requestedMem) {
    memorySegment *current = memList;
    memorySegment *best;

    while (current) {
        if (!current->occupied) {
            if (current->length == requestedMem) {
                current->occupied = true;
                return current;
            }
            if (current->length >= requestedMem) {
                if (!best) {
                    best = current;
                } else if (best->length > current->length) {
                    best = current;
                }
            }
        }
        current = current->next;
    }
    if (best) {
        current = best;
        int memoryLeft = current->length - requestedMem;
        current->occupied = true;
        current->length = requestedMem;
        if (current->next) {
            if (current->next->occupied == false) {
                current->next->startAddress = current->startAddress + requestedMem;
                current->next->length += memoryLeft;
                return current;
            }
        }
        newLen = memoryLeft;
        newAddress = current->startAddress + requestedMem;
        insertListItemAfter(current);
        return current;
    }
    return NULL;
}

memorySegment *assignNextDyn(memorySegment *memList, uint16_t requestedMem) {
    memorySegment *current;
    if (!LastBlock) {
        current = memList;
    } else {
        current = LastBlock;
    }
    while (current) {
        if (!current->occupied) {
            if (current->length > requestedMem) {
                int memoryLeft = current->length - requestedMem;
                current->occupied = true;
                current->length = requestedMem;
                if (current->next) {
                    if (!current->next->occupied) {
                        current->next->startAddress = current->startAddress + requestedMem;
                        current->next->length += memoryLeft;
                        return current;
                    }
                }
                newLen = memoryLeft;
                newAddress = current->startAddress + requestedMem;
                insertListItemAfter(current);
                return current;
            }
        }
        current = current->next;
    }
    return NULL;
}

void reclaimDyn(memorySegment *memList, memorySegment *thisOne) {
    memorySegment *currentSegment = memList;

    while (currentSegment != NULL) {
        if (currentSegment->startAddress == thisOne->startAddress) {
            currentSegment->occupied = false;
            if (currentSegment->next) {
                if (currentSegment->next->occupied == false) {
                    currentSegment->length += currentSegment->next->length;
                    currentSegment->next = currentSegment->next->next;
                }
            }
            break;
        }
        currentSegment = currentSegment->next;
    }
}