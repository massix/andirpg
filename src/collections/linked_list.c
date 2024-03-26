// AndiRPG -- Name not final
// Copyright © 2024 Massimo Gengarelli
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "collections/linked_list.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct Node {
  struct Node *_previous;
  struct Node *_next;
  void        *_content;
} Node;

struct LinkedList {
  size_t _current_size;
  size_t _filled_nodes;

  FreeFunction _free_fn;

  Node *_first;
  Node *_last;
  Node *_iterator;
};

Node *node_new() {
  Node *self = calloc(1, sizeof(Node));
  self->_next = nullptr;
  self->_previous = nullptr;
  self->_content = nullptr;

  return self;
}

bool node_is_empty(Node const *self) {
  return self->_content == nullptr;
}

bool node_has_next(Node const *self) {
  return self->_next != nullptr;
}

bool node_has_previous(Node const *self) {
  return self->_previous != nullptr;
}

Node *node_get_next(Node const *self) {
  return self->_next;
}

Node *node_get_previous(Node const *self) {
  return self->_previous;
}

void *node_get_content(Node const *self) {
  return self->_content;
}

void node_set_content(Node *self, void *content) {
  self->_content = content;
}

void node_set_previous(Node *self, Node *other) {
  self->_previous = other;
}

void node_set_next(Node *self, Node *other) {
  self->_next = other;
}

// Private method
Node *linked_list_get_first_empty(LinkedList const *self) {
  Node *current_node = self->_first;
  while (!node_is_empty(current_node) && current_node != self->_last) {
    current_node = node_get_next(current_node);
  }

  if (node_is_empty(current_node)) {
    return current_node;
  }

  return nullptr;
}

// Private method
Node *linked_list_get_last_non_empty(LinkedList const *self) {
  Node *result = nullptr;
  return result;
}

LinkedList *linked_list_new(uint32_t initial_size, FreeFunction free_fn) {
  LinkedList *self = calloc(1, sizeof(LinkedList));
  self->_current_size = initial_size;
  self->_filled_nodes = 0;
  self->_iterator = nullptr;
  self->_first = nullptr;
  self->_last = nullptr;
  self->_free_fn = free_fn;

  Node *last_added = nullptr;
  for (uint32_t i = 0; i < initial_size; i++) {
    Node *node = node_new();

    if (i == 0) {
      self->_first = node;
      self->_last = node;
    } else if (i == (initial_size - 1)) {
      self->_last = node;
      node_set_previous(node, last_added);
      node_set_next(last_added, node);
    } else {
      node_set_previous(node, last_added);
      node_set_next(last_added, node);
    }

    last_added = node;
  }

  return self;
}

void linked_list_free(LinkedList *self) {
  Node *current_node = self->_last;

  while (current_node != nullptr) {
    if (!node_is_empty(current_node) && self->_free_fn != nullptr) {
      self->_free_fn(current_node->_content);
    }

    Node *next_node = node_get_previous(current_node);
    free(current_node);

    current_node = next_node;
  }

  free(self);
}

bool linked_list_is_empty(LinkedList const *self) {
  if (self->_first == nullptr && self->_last == nullptr) {
    return true;
  }

  Node const *current_node = self->_first;

  bool all_empty = node_is_empty(current_node);
  while (node_has_next(current_node) && all_empty) {
    all_empty &= node_is_empty(current_node);
    current_node = node_get_next(current_node);
  }

  return all_empty;
}

uint32_t linked_list_get_current_size(LinkedList const *self) {
  return self->_current_size;
}

void linked_list_add(LinkedList *self, void *object) {
  if (self->_current_size == 0) {
    Node *new_node = node_new();
    self->_first = new_node;
    self->_last = new_node;
    node_set_content(new_node, object);
    self->_current_size++;
  } else if (self->_filled_nodes == self->_current_size) {
    Node *new_node = node_new();
    node_set_content(new_node, object);

    node_set_previous(new_node, self->_last);

    node_set_next(self->_last, new_node);
    self->_last = new_node;

    self->_current_size++;
  } else {
    node_set_content(linked_list_get_first_empty(self), object);
  }

  self->_filled_nodes++;
}

void linked_list_remove(LinkedList *self, uint32_t index) {
  // TODO: handle the error case in a better way
  if (linked_list_is_empty(self) || index >= self->_filled_nodes) {
    return;
  }

  // Find the node to be removed, walk the chain for `i' steps
  Node *to_be_removed = self->_first;
  for (uint32_t i = 0; i < index; i++) {
    to_be_removed = node_get_next(to_be_removed);
  }

  // Set the pointers right

  // If node has both a previous and a next, then it is in the middle of the chain
  if (node_has_previous(to_be_removed) && node_has_next(to_be_removed)) {
    node_set_next(node_get_previous(to_be_removed), node_get_next(to_be_removed));
    node_set_previous(node_get_next(to_be_removed), node_get_previous(to_be_removed));

    // Otherwise, if node only has a next, then it was the first of the chain
  } else if (node_has_next(to_be_removed)) {
    self->_first = node_get_next(to_be_removed);
    node_set_previous(self->_first, nullptr);
  }

  // If the node is not the last, then push it into last position
  if (to_be_removed != self->_last) {
    node_set_next(self->_last, to_be_removed);
    node_set_previous(to_be_removed, self->_last);
    node_set_next(to_be_removed, nullptr);

    self->_last = to_be_removed;
  }

  // Finally, empty the node
  if (self->_free_fn != nullptr) {
    self->_free_fn(to_be_removed->_content);
  }
  node_set_content(to_be_removed, nullptr);

  self->_filled_nodes--;
}

uint32_t linked_list_count(LinkedList const *self) {
  return self->_filled_nodes;
}

void *linked_list_iterator_get(LinkedList const *self) {
  if (self->_iterator == nullptr || node_is_empty(self->_iterator)) {
    return nullptr;
  }

  return node_get_content(self->_iterator);
}

bool linked_list_iterator_has_next(LinkedList const *self) {
  if (self->_iterator == nullptr && !linked_list_is_empty(self)) {
    return true;
  }

  return self->_iterator != nullptr && node_has_next(self->_iterator) && !node_is_empty(node_get_next(self->_iterator));
}

void *linked_list_iterator_next(LinkedList *self) {
  if (!linked_list_is_empty(self) && linked_list_iterator_has_next(self)) {
    if (self->_iterator != nullptr) {
      self->_iterator = node_get_next(self->_iterator);
    } else {
      self->_iterator = self->_first;
    }
    return node_get_content(self->_iterator);
  }

  return nullptr;
}

void linked_list_iterator_reset(LinkedList *self) {
  self->_iterator = nullptr;
}

void linked_list_memory_extend(LinkedList *self, uint32_t size) {
  for (uint32_t i = 0; i < size; i++) {
    Node *new_node = node_new();
    if (self->_last != nullptr) {
      node_set_next(self->_last, new_node);
      node_set_previous(new_node, self->_last);
      self->_last = new_node;
    } else {
      self->_first = new_node;
      self->_last = new_node;
    }

    self->_current_size++;
  }
}

void linked_list_memory_shrink(LinkedList *self) {
  if (self->_last == nullptr) {
    return;
  }

  Node *current = self->_last;
  while (current != nullptr && node_is_empty(current)) {
    if (node_has_previous(current)) {
      node_set_next(node_get_previous(current), nullptr);
    }

    self->_last = node_get_previous(current);
    free(current);

    current = self->_last;
    self->_current_size--;
  }

  if (self->_current_size == 0) {
    self->_first = nullptr;
    self->_last = nullptr;
  } else if (self->_current_size == 1) {
    self->_last = self->_first;
    node_set_next(self->_first, nullptr);
    node_set_previous(self->_first, nullptr);
  }
}

void *linked_list_get(LinkedList const *self, uint32_t index) {
  if (index >= self->_filled_nodes) {
    return nullptr;
  }

  Node const *current_node = self->_first;
  for (uint32_t i = 0; i < index; i++) {
    current_node = node_get_next(current_node);
  }

  return node_get_content(current_node);
}

void *linked_list_find(LinkedList const *self, Comparator comparator) {
  Node const *current_node = self->_first;

  while (!node_is_empty(current_node) && current_node != nullptr) {
    if (comparator(node_get_content(current_node))) {
      return node_get_content(current_node);
    }

    current_node = node_get_next(current_node);
  }

  return nullptr;
}

void **linked_list_find_all(LinkedList const *self, Comparator comparator, size_t *final_size) {
  *final_size = 0;

  void **result;
  result = malloc(self->_filled_nodes * sizeof(void *));

  Node const *current_node = self->_first;
  while (!node_is_empty(current_node) && current_node != nullptr) {
    if (comparator(node_get_content(current_node))) {
      result[(*final_size)++] = node_get_content(current_node);
    }

    current_node = node_get_next(current_node);
  }

  // Shrink the list
  result = realloc(result, (*final_size) + 1);
  result[*final_size] = nullptr;

  return result;
}
