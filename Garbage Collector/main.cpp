#include <stdio.h>
#include <stdlib.h>

#define STACK_MAX_SIZE 256
#define IGCT 8

typedef enum {
	INT,
	TWIN
} ObjectType;

typedef struct sObject {
	ObjectType type;
	unsigned char marked;

	struct sObject* next;

	union {
		int value;

		struct {
			struct sObject* head;
			struct sObject* tail;
		};
	};
} Object;

typedef struct {
	Object* stack[STACK_MAX_SIZE];
	int stackSize;

	Object* firstObject;

	int numObjects;

	int maxObjects;
} VM;


void push(VM* vm, Object* value) {
	vm->stack[vm->stackSize++] = value;
}


Object* pop(VM* vm) {
	return vm->stack[--vm->stackSize];
}



VM* newVM() {
	VM* vm = (VM*)malloc(sizeof(VM));
	vm->stackSize = 0;
	vm->firstObject = NULL;
	vm->numObjects = 0;
	vm->maxObjects = IGCT;
	return vm;
}


void mark(Object* object) {
	if (object->marked) return;

	object->marked = 1;

	if (object->type == TWIN) {
		mark(object->head);
		mark(object->tail);
	}
}

void markAll(VM* vm)
{
	for (int i = 0; i < vm->stackSize; i++) {
		mark(vm->stack[i]);
	}
}

void markspeep(VM* vm)
{
	Object** object = &vm->firstObject;
	while (*object) {
		if (!(*object)->marked) {
			Object* unreached = *object;

			*object = unreached->next;
			free(unreached);

			vm->numObjects--;
		}
		else {
			(*object)->marked = 0;
			object = &(*object)->next;
		}
	}
}


void gc(VM* vm) {
	int numObjects = vm->numObjects;

	markAll(vm);
	markspeep(vm);

	vm->maxObjects = vm->numObjects * 2;

	printf("Collected %d objects, %d left.\n", numObjects - vm->numObjects, vm->numObjects);
}

Object* newObject(VM* vm, ObjectType type) {
	if (vm->numObjects == vm->maxObjects) gc(vm);

	Object* object = (Object*)malloc(sizeof(Object));
	object->type = type;
	object->next = vm->firstObject;
	vm->firstObject = object;
	object->marked = 0;

	vm->numObjects++;

	return object;
}

void pushInt(VM* vm, int intValue) {
	Object* object = newObject(vm, INT);
	object->value = intValue;

	push(vm, object);
}

Object* pushPair(VM* vm) {
	Object* object = newObject(vm, TWIN);
	object->tail = pop(vm);
	object->head = pop(vm);

	push(vm, object);
	return object;
}

void objectPrint(Object* object) {
	switch (object->type) {
		case INT:
		printf("%d", object->value);
		break;

		case TWIN:
		printf("(");
		objectPrint(object->head);
		printf(", ");
		objectPrint(object->tail);
		printf(")");
		break;
	}
}

void freeVM(VM *vm) {
	vm->stackSize = 0;
	gc(vm);
	free(vm);
}

void first_test() {
	printf("1: Objects on the stack are preserved.\n");
	VM* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);

	gc(vm);
	freeVM(vm);
}

void second_test() {
	printf("2: Unreached objects are collected.\n");
	VM* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);
	pop(vm);
	pop(vm);

	gc(vm);
	freeVM(vm);
}

void third_test() {
	printf("3: Reach the nested objects.\n");
	VM* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);
	pushPair(vm);
	pushInt(vm, 3);
	pushInt(vm, 4);
	pushPair(vm);
	pushPair(vm);

	gc(vm);
	freeVM(vm);
}

void another_test() {
	printf("4: Cycles.\n");
	VM* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);
	Object* a = pushPair(vm);
	pushInt(vm, 3);
	pushInt(vm, 4);
	Object* b = pushPair(vm);

	a->tail = b;
	b->tail = a;

	gc(vm);
	freeVM(vm);
}

void performance() {
	printf("Performance of GC.\n");
	VM* vm = newVM();

	for (int i = 0; i < 1000; i++) {
		for (int j = 0; j < 20; j++) {
			pushInt(vm, i);
		}

		for (int k = 0; k < 20; k++) {
			pop(vm);
		}
	}
	freeVM(vm);
}

int main(int argc, const char** argv) {
	first_test();
	second_test();
	third_test();
	another_test();
	performance();

	return 0;
}