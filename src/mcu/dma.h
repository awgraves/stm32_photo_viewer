#pragma once
#include "registers.h"

static inline void dma_stream_disable(DMA_stream_t *stream) {
  // write is safe, won't update until all current xfers finish
  stream->CR &= ~(DMA_SxCR_EN);
  while (stream->CR & DMA_SxCR_EN)
    ;
}

static inline void dma_stream_enable(DMA_stream_t *stream) {
  stream->CR |= DMA_SxCR_EN;
  while (!(stream->CR & DMA_SxCR_EN))
    ;
}
