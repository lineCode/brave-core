// Copyright (c) 2020 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'

export default function useScrollIntoView(shouldScrollIntoView: boolean) {
  // If we need to scroll the article in to view after render,
  // do so after the element has been mounted.
  const cardRef = React.useRef<HTMLAnchorElement>(null)
  React.useEffect(() => {
    if (shouldScrollIntoView && cardRef.current) {
      cardRef.current.scrollIntoView({block: 'center'})
    }
  }, []); // empty array so only runs on first render
  return [cardRef]
}

type useScrollIntoViewReturn = [React.RefObject<HTMLAnchorElement>, () => any]

export function userScrollIntoViewAfterImagesLoaded(shouldScrollIntoView: boolean): useScrollIntoViewReturn {
  // If we need to scroll the article in to view after render,
  // do so after the image has been loaded. Assume that all the other
  // previous images are loaded and the articles are occupying
  // the size they would do with images.
  const cardRef = React.useRef<HTMLAnchorElement>(null)
  const hasScrolled = React.useRef<boolean>(false)
  const hasImageLoaded = React.useRef<boolean>(false)
  const scrollIntoViewConditionally = function () {
    if (shouldScrollIntoView && !hasScrolled.current && hasImageLoaded.current && cardRef.current) {
      hasScrolled.current = true
      cardRef.current.scrollIntoView({block: 'center'})
    }
  }
  const onImageLoaded: () => any = () => {
    hasImageLoaded.current = true
    requestAnimationFrame(() => {
      scrollIntoViewConditionally()
    })
  }
  React.useEffect(() => {
    scrollIntoViewConditionally()
  }, []); // empty array so only runs on first render
  return [cardRef, onImageLoaded]
}
