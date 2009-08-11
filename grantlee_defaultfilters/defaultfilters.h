/*
  This file is part of the Grantlee template system.

  Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 3 only, as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License version 3 for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef DEFAULTFILTERS_H
#define DEFAULTFILTERS_H

#include "node.h"
#include "filter.h"
#include "interfaces/taglibraryinterface.h"

#include "strings.h"
#include "lists.h"
#include "integers.h"
#include "datetime.h"
#include "logic.h"
#include "misc.h"

#include "grantlee_export.h"

namespace Grantlee
{
class Parser;
}

using namespace Grantlee;

class DefaultFiltersLibrary : public QObject, public TagLibraryInterface
{
  Q_OBJECT
  Q_INTERFACES( Grantlee::TagLibraryInterface )
public:
  DefaultFiltersLibrary( QObject *parent = 0 )
      : QObject( parent ) {
  }

  virtual QHash<QString, Filter*> filters( const QString &name = QString() ) {
    Q_UNUSED( name );

    QHash<QString, Filter*> filters;

    filters.insert( "upper", new UpperFilter() );
    filters.insert( "lower", new LowerFilter() );
    filters.insert( "yesno", new YesNoFilter() );
    filters.insert( "truncatewords", new TruncateWordsFilter() );
    filters.insert( "join", new JoinFilter() );
    filters.insert( "removetags", new RemoveTagsFilter() );
    filters.insert( "default_if_none", new DefaultIfNoneFilter() );
    filters.insert( "cut", new CutFilter() );
    filters.insert( "slice", new SliceFilter() );
    filters.insert( "safe", new SafeFilter() );
    filters.insert( "addslashes", new AddSlashesFilter() );
    filters.insert( "capfirst", new CapFirstFilter() );
    filters.insert( "fixampersands", new FixAmpersandsFilter() );
    filters.insert( "linenumbers", new LineNumbersFilter() );
    filters.insert( "date", new DateFilter() );
    filters.insert( "time", new TimeFilter() );
    filters.insert( "timesince", new TimeSinceFilter() );
    return filters;
  }
};

#endif

