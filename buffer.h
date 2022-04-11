#ifndef BUFFER_H
#define BUFFER_H

typedef struct Buffer {
  uint length;
  uint* contents;
} Buffer;

Buffer buffer_init(uint length) {
  Buffer buffer;
  buffer.length = length;
  buffer.contents = calloc(length, sizeof(uint));
  for (uint i = 0; i < length; ++i) {
    buffer[i] = 0;
  }

  return buffer;
}

/*
 * Append a value to the end of the buffer 
 */
void buffer_unshift(Buffer buffer, uint value) {
  for (uint i = 0; i < buffer.length - 1; ++i) {
    buffer[i] = buffer[i + 1];
  }
  buffer[buffer.length - 1] = value;
}

uint buffer_tail(Buffer buffer) {
  return buffer.contents[buffer.length - 1];
}

/*
 * Average of the contents of buffer within the specified indexes (inclusive)
 *
 */
uint buffer_average(Buffer buffer, size_t from, size_t to) {
  uint sum = 0;
  for (uint i = from; i <= to; ++i) {
    sum += buffer.contents[i];
  }

  return sum / (to - from);
}

uint buffer_average_head(Buffer buffer) {
  return buffer_average(0, buffer.length / 2 - 1);
}

uint buffer_average_tail(Buffer buffer) {
  return buffer_average(buffer.length / 2, buffer.length - 1);
}

#endif