/*******************************************************************\

Module:

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/


#ifndef CPROVER_JAVA_BYTECODE_JAVA_ENTRY_POINT_H
#define CPROVER_JAVA_BYTECODE_JAVA_ENTRY_POINT_H

#include <util/irep.h>

bool java_entry_point(
  class symbol_tablet &symbol_table,
  const irep_idt &main_class,
  class message_handlert &message_handler,
  bool assume_init_pointers_not_null,
  size_t max_nondet_array_length);

typedef struct
{
  symbolt main_function;
  bool error_found;
  bool stop_convert;
} main_function_resultt;

main_function_resultt get_main_symbol(
  symbol_tablet &symbol_table,
  const irep_idt &main_class,
  message_handlert &,
  bool allow_no_body=false);

bool generate_java_start_function(
  const symbolt &symbol,
  class symbol_tablet &symbol_table,
  class message_handlert &message_handler,
  bool assume_init_pointers_not_null,
  size_t max_nondet_array_length,
  size_t max_nondet_tree_depth,
  const select_pointer_typet &pointer_type_selector);

#endif // CPROVER_JAVA_BYTECODE_JAVA_ENTRY_POINT_H
