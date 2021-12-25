#include "ds.h"

Array<uint8_t> bits = {};
int versionSum = 0;

int ReadInt(int n) {
	int x = 0;

	for (int i = n - 1; i >= 0; i--) {
		assert(bits.Length());
		x |= bits[0] << i;
		bits.Delete(0);
	}	

	return x;
}

int64_t ConsumePacket(int indent = 0) {
	versionSum += ReadInt(3);
	int type = ReadInt(3);
	int64_t y = 0;

	if (type == 4) {
		while (true) {
			int64_t x = ReadInt(5);
			y = (y << 4) | (x & 0xF);
			if ((x & 0x10) == 0) break;
		}
	} else {
		bool mode = ReadInt(1);
		Array<int64_t> arguments = {};

		if (mode) {
			int count = ReadInt(11);
			for (int i = 0; i < count; i++) arguments.Add(ConsumePacket(indent + 1));
		} else {
			int count = ReadInt(15);
			int end = bits.Length() - count;
			while (bits.Length() > end) arguments.Add(ConsumePacket(indent + 1));
		}

		if (type == 0) {
			for (int i = 0; i < arguments.Length(); i++) y += arguments[i];
		} else if (type == 1) {
			y = 1;
			for (int i = 0; i < arguments.Length(); i++) y *= arguments[i];
		} else if (type == 2) {
			y = LONG_MAX;
			for (int i = 0; i < arguments.Length(); i++) MINIMIZE(y, arguments[i]);
		} else if (type == 3) {
			y = LONG_MIN;
			for (int i = 0; i < arguments.Length(); i++) MAXIMIZE(y, arguments[i]);
		} else if (type == 5) {
			y = arguments[0] > arguments[1] ? 1 : 0;
		} else if (type == 6) {
			y = arguments[0] < arguments[1] ? 1 : 0;
		} else if (type == 7) {
			y = arguments[0] == arguments[1] ? 1 : 0;
		}

		arguments.Free();
	}

	return y;
}

int main() {
	FILE *f = fopen("in16.txt", "rb");

	while (true) {
		int c = fgetc(f);
		if (c >= 'A' && c <= 'F') c = c - 'A' + 10;
		else if (c >= '0' && c <= '9') c = c - '0';
		else break;
		bits.Add((c & (1 << 3)) ? 1 : 0);
		bits.Add((c & (1 << 2)) ? 1 : 0);
		bits.Add((c & (1 << 1)) ? 1 : 0);
		bits.Add((c & (1 << 0)) ? 1 : 0);
	}

	int64_t y = ConsumePacket();
	bits.Free();
	fclose(f);
	printf("part 1: %d\n", versionSum);
	printf("part 2: %ld\n", y);
}
