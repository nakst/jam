typedef struct PNGReader {
	const void *buffer;
	size_t bytes;
	uintptr_t position;
	uintptr_t idatBytesRemaining;
	const uint8_t *idatPointer;
	bool error;
	uint8_t zlibByte, zlibBitsRemaining;
} PNGReader;

uint32_t SwapBigEndian32(uint32_t x) {
	return    ((x & 0xFF000000) >> 24) 
		| ((x & 0x000000FF) << 24) 
		| ((x & 0x00FF0000) >> 8) 
		| ((x & 0x0000FF00) << 8);
}

const void *PNGRead(PNGReader *reader, size_t bytes) {
	if (bytes > reader->bytes || reader->position > reader->bytes - bytes || reader->error) {
		reader->error = true;
		return NULL;
	} else {
		reader->position += bytes;
		return (const uint8_t *) reader->buffer + reader->position - bytes;
	}
}

const uint8_t *PNGReadIDATByte(PNGReader *reader) {
	if (!reader->idatBytesRemaining) {
		uintptr_t position = reader->position;

		while (true) {
			const uint32_t *chunkSize = (const uint32_t *) PNGRead(reader, sizeof(uint32_t));
			const uint32_t *chunkType = (const uint32_t *) PNGRead(reader, sizeof(uint32_t));

			if (!chunkSize || !chunkType) {
				return NULL;
			}

			const void *chunkData = PNGRead(reader, SwapBigEndian32(*chunkSize));
			const uint32_t *chunkChecksum = (const uint32_t *) PNGRead(reader, sizeof(uint32_t));

			if (!chunkData || !chunkChecksum) {
				return NULL;
			}

			if (SwapBigEndian32(*chunkType) == 0x49454E44) {
				reader->position = position;
				break;
			} else if (SwapBigEndian32(*chunkType) != 0x49444154) {
				continue;
			}

			reader->idatBytesRemaining = SwapBigEndian32(*chunkSize);
			reader->idatPointer = (const uint8_t *) chunkData;
			break;
		}
	}

	if (!reader->idatBytesRemaining) {
		reader->error = true;
		return NULL;
	}

	const uint8_t *pointer = reader->idatPointer;
	reader->idatPointer++;
	reader->idatBytesRemaining--;
	return pointer;
}

uint32_t PNGReadZLIBBits(PNGReader *reader, uint8_t count) {
	uint32_t result = 0;
	uintptr_t shift = 0;

	while (count) {
		if (!reader->zlibBitsRemaining) {
			const uint8_t *nextByte = PNGReadIDATByte(reader);
			if (!nextByte) return 0;
			reader->zlibByte = *nextByte;
			reader->zlibBitsRemaining = 8;
		}

		result |= (reader->zlibByte & 1) << shift;
		reader->zlibByte >>= 1;
		reader->zlibBitsRemaining--;
		count--, shift++;
	}

	return result;
}

uint32_t PNGReadHuffman(PNGReader *reader, uint32_t *tree, size_t treeEntries) {
	uint16_t code = 0;
	uint16_t length = 0;

	while (true) {
		code <<= 1;
		code |= PNGReadZLIBBits(reader, 1);
		length++;

		if (code >= treeEntries) {
			reader->error = true;
			return 0;
		}

		uint32_t entry = tree[code];

		if ((entry >> 16) == length) {
			return entry & 0xFFFF;
		}
	}
}

bool PNGBuildHuffmanTree(uint16_t count, uint8_t *lengths, uint32_t *tree, size_t treeSize) {
	uint16_t lengthsCount[16] = {};

	for (uintptr_t i = 0; i < count; i++) {
		lengthsCount[lengths[i]]++;
	}

	uint16_t code = 0;
	uint16_t nextCodes[16] = {};
	lengthsCount[0] = 0;

	for (uintptr_t length = 1; length < 16; length++) {
		code = (code + lengthsCount[length - 1]) << 1;
		if (lengthsCount[length]) nextCodes[length] = code;
	}

	for (uintptr_t i = 0; i < count; i++) {
		uint8_t length = lengths[i];
		if (!length) continue;
		uint16_t code = nextCodes[length]++;
		if (code >= treeSize) return false;
		tree[code] = i | (length << 16);
	}

	return true;
}

typedef struct PNGImageHeader {
	uint32_t width, height;
	uint8_t bitDepth, colorType;
	uint8_t compressionMethod, filterMethod, interlaceMethod;
} PNGImageHeader;

const uint8_t pngExtraLengthBits[] = { 
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1,  
	1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
	4, 4, 4, 4, 5, 5, 5, 5, 0
};

const uint16_t pngExtraLengthOffsets[] = {
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13,
	15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
	67, 83, 99, 115, 131, 163, 195, 227, 258,
};

const uint8_t pngExtraDistanceBits[] = { 
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3,
	4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
	9, 9, 10, 10, 11, 11, 12, 12, 13, 13,
};

const uint16_t pngExtraDistanceOffsets[] = {
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25,
	33, 49, 65, 97, 129, 193, 257, 385, 513, 769,
	1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577,
};

const uint8_t pngReorderCLCLs[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

bool PNGParse(PNGReader *reader, uint32_t **outBits, int32_t *outWidth, int32_t *outHeight, void *(*alloc)(size_t)) {
	// TODO Validate checksums.

	// Check the signature is present.

	const uint64_t *signature = (const uint64_t *) PNGRead(reader, sizeof(uint64_t));

	if (!signature || *signature != ((uint64_t) 0x0A1A0A0D474E5089)) {
		return false;
	}

	// Read the IHDR chunk.

	const uint32_t *ihdrSize = (const uint32_t *) PNGRead(reader, sizeof(uint32_t));
	const uint32_t *ihdrType = (const uint32_t *) PNGRead(reader, sizeof(uint32_t));

	if (!ihdrSize || !ihdrType || SwapBigEndian32(*ihdrSize) < 13 || SwapBigEndian32(*ihdrType) != 0x49484452) {
		return false;
	}

	const void *ihdrData = PNGRead(reader, SwapBigEndian32(*ihdrSize));
	const uint32_t *ihdrChecksum = (const uint32_t *) PNGRead(reader, sizeof(uint32_t));

	if (!ihdrData || !ihdrChecksum) {
		return false;
	}

	PNGImageHeader imageHeader = *(PNGImageHeader *) ihdrData;

	uint32_t width = SwapBigEndian32(imageHeader.width);
	uint32_t height = SwapBigEndian32(imageHeader.height);
	uint8_t bytesPerPixel = 0;
	uint32_t bytesPerScanline;
	bool usesPalette = false;
	uint32_t palette[256];

	// Check the image is supported.

	if (width > 16383 || height > 16383 || imageHeader.compressionMethod || imageHeader.filterMethod || imageHeader.interlaceMethod) {
		return false;
	}
			
	if (imageHeader.colorType == 0 && imageHeader.bitDepth == 8) {
		bytesPerPixel = 1;
		bytesPerScanline = 1 * width;
	} else if (imageHeader.colorType == 2 && imageHeader.bitDepth == 8) {
		bytesPerPixel = 3;
		bytesPerScanline = 3 * width;
	} else if (imageHeader.colorType == 6 && imageHeader.bitDepth == 8) {
		bytesPerPixel = 4;
		bytesPerScanline = 4 * width;
	} else if (imageHeader.colorType == 3 && (imageHeader.bitDepth == 1 || imageHeader.bitDepth == 2 
				|| imageHeader.bitDepth == 4 || imageHeader.bitDepth == 8)) {
		bytesPerPixel = 1;
		usesPalette = true;
		bytesPerScanline = (imageHeader.bitDepth * width + 7) >> 3;
	} else {
		return false;
	}

	// Read the PLTE and tRNS chunk for paletted images.

	if (usesPalette) {
		uintptr_t position = reader->position;
		bool foundPalette = false;

		for (uint32_t i = 0; i < 256; i++) {
			palette[i] = 0xFF000000;
		}

		while (true) {
			const uint32_t *chunkSize = (const uint32_t *) PNGRead(reader, sizeof(uint32_t));
			const uint32_t *chunkType = (const uint32_t *) PNGRead(reader, sizeof(uint32_t));

			if (!chunkSize || !chunkType) {
				return false;
			}

			uint32_t chunkSize0 = SwapBigEndian32(*chunkSize);
			const uint8_t *chunkData = (uint8_t *) PNGRead(reader, chunkSize0);
			const uint32_t *chunkChecksum = (const uint32_t *) PNGRead(reader, sizeof(uint32_t));

			if (!chunkData || !chunkChecksum) {
				return false;
			}

			if (SwapBigEndian32(*chunkType) == 0x49454E44) {
				break;
			} else if (SwapBigEndian32(*chunkType) == 0x504C5445) {
				if ((chunkSize0 % 3) || chunkSize0 > 256 * 3) {
					return false;
				}

				for (uint32_t i = 0; i < 256 && i * 3 < chunkSize0; i++) {
					palette[i] = (palette[i] & 0xFF000000) | (chunkData[i * 3 + 0] << 0) 
						| (chunkData[i * 3 + 1] << 8) | (chunkData[i * 3 + 2] << 16);
				}

				foundPalette = true;
			} else if (SwapBigEndian32(*chunkType) == 0x74524E53) {
				if (chunkSize0 > 256) {
					return false;
				}

				for (uint32_t i = 0; i < chunkSize0; i++) {
					palette[i] = (palette[i] & 0x00FFFFFF) | ((uint32_t) chunkData[i] << 24);
				}
			}
		}

		reader->position = position;
		if (!foundPalette) return false;
	}

	// Check the ZLIB header.

	const uint8_t *zlibCMF = (const uint8_t *) PNGReadIDATByte(reader);
	const uint8_t *zlibFLG = (const uint8_t *) PNGReadIDATByte(reader);

	if (!zlibCMF || !zlibFLG 
			|| (*zlibCMF & 0x0F) != 0x08
			|| (*zlibCMF & 0xF0) > 0x70
			|| (*zlibFLG & 0x20)) {
		return false;
	}

	// Decode the ZLIB blocks.

	size_t bufferSize = (bytesPerScanline + 1) * height;
	uint8_t *buffer = (uint8_t *) alloc(bufferSize + 262144);

	if (!buffer) {
		return false;
	}

	uintptr_t bufferPosition = 0;
	bool isFinalBlock = false;

	uint32_t *litTree = (uint32_t *) (buffer + bufferSize);
	uint32_t *distTree = (uint32_t *) (buffer + bufferSize + 131072);

	while (!reader->error && !isFinalBlock) {
		isFinalBlock = PNGReadZLIBBits(reader, 1);
		uint8_t compressionType = PNGReadZLIBBits(reader, 2);
		
		if (compressionType == 0) {
			// Uncompressed data.

			PNGReadZLIBBits(reader, reader->zlibBitsRemaining);
			const uint8_t *lengthHigh = PNGReadIDATByte(reader);
			const uint8_t *lengthLow = PNGReadIDATByte(reader);
			if (!lengthHigh || !lengthLow) break;
			uint16_t length = (*lengthHigh << 8) | *lengthLow;
			PNGReadIDATByte(reader);
			PNGReadIDATByte(reader);

			for (uintptr_t i = 0; i < length; i++) {
				const uint8_t *byte = PNGReadIDATByte(reader);
				if (!byte) break;
				
				if (bufferPosition == bufferSize) {
					reader->error = true;
					break;
				}

				buffer[bufferPosition++] = *byte;
			}

			continue;
		} else if (compressionType == 3) {
			// Invalid compression type.

			reader->error = true;
			break;
		}

		// Compressed data.

		memset(buffer + bufferSize, 0, 262144);

		if (compressionType == 2) {
			// Read the Huffman code.

			uint16_t hlit = PNGReadZLIBBits(reader, 5) + 257;
			uint16_t hdist = PNGReadZLIBBits(reader, 5) + 1;
			uint16_t hclen = PNGReadZLIBBits(reader, 4) + 4;

			uint32_t codeLengthTree[128] = {};

			if (!reader->error) {
				uint8_t lengths[19] = {};

				for (uintptr_t i = 0; i < hclen && !reader->error; i++) {
					uint8_t length = PNGReadZLIBBits(reader, 3);
					lengths[pngReorderCLCLs[i]] = length;
				}

				if (!PNGBuildHuffmanTree(19, lengths, codeLengthTree, 128)) {
					reader->error = true;
				}
			}

			if (!reader->error) {
				uint8_t lengths[286 + 32] = {};

				for (uintptr_t i = 0; i < (hlit + hdist) && !reader->error; i++) {
					uint32_t symbol = PNGReadHuffman(reader, codeLengthTree, 128);
					
					if (symbol < 16) {
						lengths[i] = symbol;
						continue;
					} 

					uint8_t repeatValue;
					uint8_t repeatReadBits;
					size_t repeat;

					if (symbol == 16) {
						if (!i) {
							reader->error = true;
							break;
						}

						repeatValue = lengths[i - 1];
						repeatReadBits = 2;
						repeat = 3;
					} else {
						repeatValue = 0;
						repeatReadBits = symbol == 17 ? 3 : 7;
						repeat = symbol == 17 ? 3 : 11;
					}

					repeat += PNGReadZLIBBits(reader, repeatReadBits);

					if (i + repeat > hlit + hdist) {
						reader->error = true;
						break;
					}

					for (uintptr_t j = 0; j < repeat; j++) {
						lengths[i + j] = repeatValue;
					}

					i += repeat - 1;
				}

				if (!PNGBuildHuffmanTree(hlit, lengths + 0, litTree, 32768) 
						|| !PNGBuildHuffmanTree(hdist, lengths + hlit, distTree, 32768)) {
					reader->error = true;
				}
			}
		} else {
			uint8_t lengths[288] = {};
			for (int i =   0; i <= 143; i++) lengths[i] = 8;
			for (int i = 144; i <= 255; i++) lengths[i] = 9;
			for (int i = 256; i <= 279; i++) lengths[i] = 7;
			for (int i = 280; i <= 287; i++) lengths[i] = 8;
			if (!PNGBuildHuffmanTree(288, lengths, litTree, 32768)) reader->error = true;
			for (int i = 0; i <= 31; i++) lengths[i] = 5;
			if (!PNGBuildHuffmanTree(32, lengths, distTree, 32768)) reader->error = true;
		}

		// Decode the data.

		while (!reader->error) {
			uint32_t symbol = PNGReadHuffman(reader, litTree, 32768);

			if (symbol < 256) {
				if (bufferPosition == bufferSize) {
					reader->error = true;
					break;
				}

				buffer[bufferPosition++] = symbol;
			} else if (symbol == 256) {
				break;
			} else if (symbol < 286) {
				uint32_t length = pngExtraLengthOffsets[symbol - 257] + PNGReadZLIBBits(reader, pngExtraLengthBits[symbol - 257]);
				symbol = PNGReadHuffman(reader, distTree, 32768);
				uint32_t distance = pngExtraDistanceOffsets[symbol] + PNGReadZLIBBits(reader, pngExtraDistanceBits[symbol]);

				if (distance > bufferPosition || !distance) {
					reader->error = true;
				} else {
					if (bufferPosition + length > bufferSize) {
						reader->error = true;
						break;
					}

					for (uintptr_t i = 0; i < length; i++, bufferPosition++) {
						buffer[bufferPosition] = buffer[bufferPosition - distance];
					}
				}
			} else {
				reader->error = true;
			}
		}
	}

	if (reader->error || bufferPosition != bufferSize) {
		return false;
	}

	// Defilter the image.

	for (uintptr_t i = 0; i < height; i++) {
		uint8_t type = buffer[i * (bytesPerScanline + 1)];
		if (!type) continue;

		for (uintptr_t j = 0; j < bytesPerScanline; j++) {
			uintptr_t k = i * (bytesPerScanline + 1) + j + 1;
			uint32_t x = buffer[k];
			uint32_t a = j >= bytesPerPixel ? buffer[k - bytesPerPixel] : 0;
			uint32_t b = i ? buffer[k - (bytesPerScanline + 1)] : 0;
			uint32_t c = i && j >= bytesPerPixel ? buffer[k - (bytesPerScanline + bytesPerPixel + 1)] : 0;

			if (type == 1) {
				buffer[k] = x + a;
			} else if (type == 2) {
				buffer[k] = x + b;
			} else if (type == 3) {
				buffer[k] = x + ((a + b) >> 1);
			} else if (type == 4) {
				int32_t p = a + b - c;
				int32_t pa = p > (int32_t) a ? p - a : a - p;
				int32_t pb = p > (int32_t) b ? p - b : b - p;
				int32_t pc = p > (int32_t) c ? p - c : c - p;
				if (pa <= pb && pa <= pc) buffer[k] = x + a;
				else if (pb <= pc) buffer[k] = x + b;
				else buffer[k] = x + c;
			} else {
				return false;
			}
		}
	}

	// Convert the image to the desired format.

	uint32_t *bits = (uint32_t *) alloc(width * height * 4);

	if (!bits) {
		return false;
	}

	if (imageHeader.colorType == 0) {
		for (uintptr_t i = 0; i < height; i++) {
			for (uintptr_t j = 0; j < width; j++) {
				uint8_t x = buffer[i * (width + 1) + j + 1];
				bits[i * width + j] = 0xFF000000 | (x << 16) | (x << 8) | x;
			}
		}
	} else if (imageHeader.colorType == 2) {
		for (uintptr_t i = 0; i < height; i++) {
			for (uintptr_t j = 0; j < width; j++) {
				uint8_t r = buffer[i * (width * 3 + 1) + j * 3 + 1];
				uint8_t g = buffer[i * (width * 3 + 1) + j * 3 + 2];
				uint8_t b = buffer[i * (width * 3 + 1) + j * 3 + 3];
				bits[i * width + j] = 0xFF000000 | (b << 16) | (g << 8) | r;
			}
		}
	} else if (imageHeader.colorType == 6) {
		for (uintptr_t i = 0; i < height; i++) {
			for (uintptr_t j = 0; j < width; j++) {
				uint8_t r = buffer[i * (width * 4 + 1) + j * 4 + 1];
				uint8_t g = buffer[i * (width * 4 + 1) + j * 4 + 2];
				uint8_t b = buffer[i * (width * 4 + 1) + j * 4 + 3];
				uint8_t a = buffer[i * (width * 4 + 1) + j * 4 + 4];
				bits[i * width + j] = (a << 24) | (b << 16) | (g << 8) | r;
			}
		}
	} else if (usesPalette && imageHeader.bitDepth == 8) {
		for (uintptr_t i = 0; i < height; i++) {
			for (uintptr_t j = 0; j < width; j++) {
				uint8_t x = buffer[i * (bytesPerScanline + 1) + j + 1];
				bits[i * width + j] = palette[x];
			}
		}
	} else if (usesPalette && imageHeader.bitDepth == 4) {
		for (uintptr_t i = 0; i < height; i++) {
			for (uintptr_t j = 0; j < width; j++) {
				uint8_t x = buffer[i * (bytesPerScanline + 1) + (j >> 1) + 1];
				bits[i * width + j] = palette[(x >> ((1 - (j & 1)) << 2)) & 15];
			}
		}
	} else if (usesPalette && imageHeader.bitDepth == 2) {
		for (uintptr_t i = 0; i < height; i++) {
			for (uintptr_t j = 0; j < width; j++) {
				uint8_t x = buffer[i * (bytesPerScanline + 1) + (j >> 2) + 1];
				bits[i * width + j] = palette[(x >> ((3 - (j & 3)) << 1)) & 3];
			}
		}
	} else if (usesPalette && imageHeader.bitDepth == 1) {
		for (uintptr_t i = 0; i < height; i++) {
			for (uintptr_t j = 0; j < width; j++) {
				uint8_t x = buffer[i * (bytesPerScanline + 1) + (j >> 3) + 1];
				bits[i * width + j] = palette[(x >> ((7 - (j & 7)) << 0)) & 1];
			}
		}
	}

	// Return the loaded data.

	*outBits = bits;
	*outWidth = width;
	*outHeight = height;

	return true;
}
