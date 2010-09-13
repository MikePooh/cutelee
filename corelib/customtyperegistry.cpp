/*
  This file is part of the Grantlee template system.

  Copyright (c) 2010 Michael Jansen <kde@michael-jansen.biz>
  Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either version
  2.1 of the Licence, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "customtyperegistry_p.h"

#include "metaenumvariable_p.h"
#include "safestring.h"

#include <QtCore/QDebug>
#include <QtCore/QStringList>

using namespace Grantlee;

namespace Grantlee
{

template<>
QVariantList VariantToList<MetaEnumVariable>::doConvert( const QVariant &obj )
{
  const MetaEnumVariable mev = obj.value<MetaEnumVariable>();

  if (mev.value != -1)
    return QVariantList();

  QVariantList list;
  for (int row = 0; row < mev.enumerator.keyCount(); ++row) {
    list << QVariant::fromValue( MetaEnumVariable( mev.enumerator, row ) );
  }
  return list;
}
}

CustomTypeRegistry::CustomTypeRegistry()
{
  // Qt Types
  registerBuiltInMetatype<QStringList>();
  registerBuiltInMetatype<QVariantList>();
  registerBuiltInMetatype<QVariantHash>();
  registerBuiltInMetatype<QObject*>();
  registerSequentialToList<QStringList>();
  registerSequentialToList<QVariantList>();

  registerAssociativeToList<QVariantHash>();

  // Grantlee Types
  registerBuiltInMetatype<SafeString>();
  registerBuiltInMetatype<MetaEnumVariable>();
  registerVariantToList<MetaEnumVariable>();
}

void CustomTypeRegistry::registerLookupOperator( int id, MetaType::LookupFunction f )
{
  mutex.lock();

  CustomTypeInfo &info = types[id];
  info.lookupFunction = f;

  mutex.unlock();
}

void CustomTypeRegistry::registerToListOperator( int id, MetaType::ToVariantListFunction f )
{
  mutex.lock();

  CustomTypeInfo &info = types[id];
  info.toVariantListFunction = f;

  mutex.unlock();
}

QVariant CustomTypeRegistry::lookup( const QVariant &object, const QString &property ) const
{
  if ( !object.isValid() )
    return QVariant();
  const int id = object.userType();
  MetaType::LookupFunction lf;

  {
    if ( !types.contains( id ) ) {
      qWarning() << "Don't know how to handle metatype" << QMetaType::typeName( id );
      // :TODO: Print out error message
      return QVariant();
    }

    const CustomTypeInfo &info = types[id];
    if ( !info.lookupFunction ) {
      qWarning() << "No lookup function for metatype" << QMetaType::typeName( id );
      lf = 0;
      // :TODO: Print out error message
      return QVariant();
    }

    lf = info.lookupFunction;
  }

  return lf( object, property );
}

QVariantList CustomTypeRegistry::toVariantList( const QVariant &variant ) const
{
  if ( !variant.isValid() )
    return QVariantList();

  const int id = variant.userType();
  MetaType::ToVariantListFunction tvlf;

  {
    if ( !types.contains( id ) ) {
      qWarning() << "Don't know how to handle metatype for list" << QMetaType::typeName( id );
      // :TODO: Print out error message
      return QVariantList();
    }

    const CustomTypeInfo &info = types[id];
    if ( !info.toVariantListFunction ) {
      qWarning() << "No toList function for metatype" << QMetaType::typeName( id );
      tvlf = 0;
      // :TODO: Print out error message
      return QVariantList();
    }

    tvlf = info.toVariantListFunction;
  }

  return tvlf( variant );
}

bool CustomTypeRegistry::lookupAlreadyRegistered( int id ) const
{
  return types.contains( id ) && types.value( id ).lookupFunction != 0;
}
