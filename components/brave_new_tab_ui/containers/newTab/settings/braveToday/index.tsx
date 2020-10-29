// Copyright (c) 2020 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'
import AutoSizer from "react-virtualized-auto-sizer";
import { VariableSizeList } from "react-window";
import { isPublisherContentAllowed } from '../../../../../common/braveToday'

// Components
import { Button } from 'brave-ui'
import { CaratRightIcon } from 'brave-ui/components/icons'
import {
  SettingsRow,
  SettingsText,
  SettingsSectionTitle,
} from '../../../../components/default'
import { Toggle } from '../../../../components/toggle'
import NavigateBack from '../../../../components/default/settings/navigateBack';
import * as Styled from './style'

interface Props {
  publishers?: BraveToday.Publishers
  setPublisherPref: (publisherId: string, enabled: boolean) => any
  onDisplay: () => any
  onClearPrefs: () => any
  showToday: boolean
  toggleShowToday: () => any
}

export const DynamicListContext = React.createContext<
  Partial<{ setSize: (index: number, size: number) => void }>
>({});

type ListItemProps = {
  index: number
  width: number
  data: BraveToday.Publisher[]
  style: React.CSSProperties
  setPublisherPref: (publisherId: string, enabled: boolean) => any
}

// Component for each item. Measures height to let virtual
// list know.
function ListItem (props: ListItemProps) {
  const { setSize } = React.useContext(DynamicListContext);
  const rowRoot = React.useRef<null | HTMLDivElement>(null);

  React.useEffect(() => {
    if (rowRoot.current) {
      setSize && setSize(props.index, rowRoot.current.getBoundingClientRect().height);
    }
  }, [props.index, setSize, props.width]);

  const publisher = props.data[props.index]
  if (!publisher) {
    console.warn('Publisher was null at index', props.index, props.data, props)
    return null
  }
  const isChecked = isPublisherContentAllowed(publisher)

  return (
    <div style={props.style}>
      <SettingsRow innerRef={rowRoot} key={publisher.publisher_id}>
        <SettingsText>{publisher.publisher_name}</SettingsText>
        <Toggle
          checked={isChecked}
          onChange={() => props.setPublisherPref(publisher.publisher_id, !isChecked)}
          size='large'
        />
      </SettingsRow>
    </div>
  )
}

type PublisherPrefsProps = {
  setPublisherPref: (publisherId: string, enabled: boolean) => any
  publishers: BraveToday.Publisher[]
}

function PublisherPrefs (props: PublisherPrefsProps) {
  const listRef = React.useRef<VariableSizeList | null>(null);
  const sizeMap = React.useRef<{ [key: string]: number }>({});

  const setSize = React.useCallback((index: number, size: number) => {
    // Performance: Only update the sizeMap and reset cache if an actual value changed
    if (sizeMap.current[index] !== size) {
      sizeMap.current = { ...sizeMap.current, [index]: size };
      if (listRef.current) {
        // Clear cached data and rerender
        listRef.current.resetAfterIndex(0);
      }
    }
  }, []);

  const getSize = React.useCallback((index) => {
    return sizeMap.current[index] || 900;
  }, []);

  const publishers = props.publishers

  return (
    <DynamicListContext.Provider
      value={{ setSize }}
    >
      <AutoSizer>
        {function ({width, height}) {
          return (
            <VariableSizeList
              ref={listRef}
              width={width}
              height={height}
              itemData={publishers}
              itemCount={publishers.length}
              itemSize={getSize}
              overscanCount={38}
              style={{
                overscrollBehavior: 'contain'
              }}
            >
              {function (itemProps) {
                return (
                  <ListItem
                    {...itemProps}
                    width={width}
                    setPublisherPref={props.setPublisherPref}
                  />
                )
              }}
            </VariableSizeList>
          )
        }}
      </AutoSizer>
    </DynamicListContext.Provider>
  )
}


type CategoryListProps = {
  categories: string[]
  setCategory: (category: string) => any
}

function CategoryList (props: CategoryListProps) {
  return (<>
    <SettingsSectionTitle>Sources</SettingsSectionTitle>
    {props.categories.map(category => {
      return (
        <SettingsRow isInteractive={true} onClick={() => props.setCategory(category)}>
          <SettingsText>{category}</SettingsText>
          <Styled.SourcesCommandIcon>
            <CaratRightIcon />
          </Styled.SourcesCommandIcon>
        </SettingsRow>
      )
    })}
  </>)
}

type CategoryProps = {
  category: string
  publishers: BraveToday.Publisher[]
  onBack: () => any
  setPublisherPref: (publisherId: string, enabled: boolean) => any
}

function Category (props: CategoryProps) {
  return (
    <Styled.Section>
      <Styled.StaticPrefs>
        <NavigateBack onBack={props.onBack}></NavigateBack>
        <SettingsSectionTitle>{props.category}</SettingsSectionTitle>
      </Styled.StaticPrefs>
      <Styled.PublisherList>
        <PublisherPrefs
          publishers={props.publishers}
          setPublisherPref={props.setPublisherPref}
        />
      </Styled.PublisherList>
    </Styled.Section>
  )
}

// TODO: l10n
const categoryNameAll = 'All sources'

type SourcesProps = Props & {
  category: string
  setCategory: (category: string) => any
}

function Sources (props: SourcesProps) {
  const publishersByCategory = React.useMemo<Map<string, BraveToday.Publisher[]>>(() => {
    const result = new Map<string, BraveToday.Publisher[]>()
    result.set(categoryNameAll, [])
    if (!props.publishers) {
      return result
    }
    for (const publisher of Object.values(props.publishers)) {
      const forAll = result.get(categoryNameAll) || []
      forAll.push(publisher)
      result.set(categoryNameAll, forAll)
      if (publisher.category) {
        const forCategory = result.get(publisher.category) || []
        forCategory.push(publisher)
        result.set(publisher.category, forCategory)
      }
    }
    // Sort all publishers alphabetically
    for (const publishers of result.values()) {
      publishers.sort((a, b) => a.publisher_name.toLocaleLowerCase().localeCompare(b.publisher_name.toLocaleLowerCase()))
    }
    return result
  }, [props.publishers])
  // No publishers
  // TODO(petemill): error state
  if (!props.publishers) {
    return <div>Loading...</div>
  }
  // Category list
  if (!props.category) {
    return <CategoryList
      categories={[...publishersByCategory.keys()]}
      setCategory={props.setCategory}
    />
  }
  const categoryPublishers = publishersByCategory.get(props.category)
  if (!categoryPublishers) {
    return null
  }
  // Category
  return <Category
    category={props.category}
    publishers={categoryPublishers}
    onBack={() => props.setCategory('')}
    setPublisherPref={props.setPublisherPref}
  />
}

export default function BraveTodayPrefs (props: Props) {
  // Ensure publishers data is fetched, which won't happen
  // if user has not interacted with Brave Today on this page
  // view.
  React.useEffect(() => {
    if (props.showToday) {
      props.onDisplay()
    }
  }, [props.onDisplay, props.showToday])

  const [category, setCategory] = React.useState<string>('')

  const confirmAction = React.useCallback(() => {
    if (confirm('Reset all your Brave Today publisher choices to their default?')) {
      props.onClearPrefs()
    }
  }, [props.onClearPrefs])

  return (
    <Styled.Section>
        {!category && <>
        <SettingsRow>
          <SettingsText>Show Brave Today</SettingsText>
          <Toggle
            checked={props.showToday}
            onChange={props.toggleShowToday}
            size='large'
          />
        </SettingsRow>
        <SettingsRow>
          <div></div>
          <Button type="warn" level="tertiary" onClick={confirmAction} text="Reset Brave Today settings" />
        </SettingsRow>
        </>}
      {props.showToday &&
      <Sources category={category} setCategory={setCategory} {...props} />
      }
    </Styled.Section>
  )
}