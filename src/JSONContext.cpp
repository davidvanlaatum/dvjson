#include "JSONContext.h"
#include <assert.h>

using namespace dv::json;

thread_local JSONContext *JSONContext::currentContext = nullptr;

JSONContext::ContextRestore::~ContextRestore() {
  auto ctx = currentContext;
  ctx->currentContext = ctx->previous;
  ctx->previous = nullptr;
}

void JSONContext::enter( const std::function<void()> &function ) {
  assert( previous == nullptr );
  assert( currentContext != this );
  ContextRestore cr __attribute__((unused));
  previous = currentContext;
  currentContext = this;
  function();
}

JSONContext::ContextRestore JSONContext::enter() {
  assert( previous == nullptr );
  assert( currentContext != this );
  ContextRestore cr;
  previous = currentContext;
  currentContext = this;
  return cr;
}
