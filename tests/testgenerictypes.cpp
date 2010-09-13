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

#include "metatype.h"
#include "engine.h"
#include "grantlee_paths.h"
#include "template.h"

#include <QtTest/QtTest>
#include <QtCore/QVariant>
#include <QtCore/QVariantHash>
#include <QtCore/QMetaType>

#include <string>

Q_DECLARE_METATYPE(QVector<QVariant>)
Q_DECLARE_METATYPE(QStack<QVariant>)
Q_DECLARE_METATYPE(QQueue<QVariant>)
Q_DECLARE_METATYPE(QLinkedList<QVariant>)

class TestGenericTypes : public QObject
{
  Q_OBJECT

private Q_SLOTS:

  void initTestCase();

  void testGenericClassType();
  void testSequentialContainer_Variant();
  void testAssociativeContainer_Variant();

}; // class TestGenericTypes

class Person
{
public:
  Person() :age(0) {}
  Person(std::string _name, int _age)
    : name(_name), age(_age)
  {

  }

 std::string name;
 int age;
};

Q_DECLARE_METATYPE(Person)

GRANTLEE_BEGIN_LOOKUP(Person)
  if ( property == QLatin1String( "name" ) )
    return QString::fromStdString( object.name );
  else if ( property == QLatin1String( "age" ) )
    return object.age;
GRANTLEE_END_LOOKUP

void TestGenericTypes::initTestCase()
{
  // Register the handler for our custom type
  Grantlee::registerMetaType<Person>();
}

void TestGenericTypes::testGenericClassType()
{
  Grantlee::Engine engine;

  engine.setPluginPaths( QStringList() << QLatin1String( GRANTLEE_PLUGIN_PATH ) );

  Grantlee::Template t1 = engine.newTemplate(
      QLatin1String( "Person: \nName: {{p.name}}\nAge: {{p.age}}\nUnknown: {{p.unknown}}" ),
      QLatin1String( "template1" ) );

  // Check it
  QVariantHash h;
  Person p( "Grant Lee", 2);
  h.insert( QLatin1String( "p" ), QVariant::fromValue(p) );
  Grantlee::Context c( h );
  QCOMPARE(
      t1->render( &c ),
      QLatin1String( "Person: \nName: Grant Lee\nAge: 2\nUnknown: " ));

}

static QMap<int, Person> getPeople()
{
  QMap<int, Person> people;
  people.insert( 23, Person( "Claire", 23 ) );
  people.insert( 32, Person( "Grant", 32) );
  people.insert( 50, Person( "Alan", 50 ) );
  return people;
}

template<typename SequentialContainer>
void insertPeopleVariants(Grantlee::Context &c)
{
  QMap<int, Person> people = getPeople();
  QMap<int, Person>::const_iterator it = people.constBegin();
  const QMap<int, Person>::const_iterator end = people.constEnd();
  SequentialContainer container;
  for ( ; it != end; ++it )
    container.push_back( QVariant::fromValue( it.value() ) );
  c.insert( QLatin1String( "people" ), QVariant::fromValue( container ) );
}

template<typename AssociativeContainer>
void insertAssociatedPeopleVariants(Grantlee::Context &c)
{
  QMap<int, Person> people = getPeople();
  QMap<int, Person>::const_iterator it = people.constBegin();
  const QMap<int, Person>::const_iterator end = people.constEnd();
  AssociativeContainer container;
  for ( ; it != end; ++it )
    container.insert( QString::number( it.key() ), QVariant::fromValue( it.value() ) );
  c.insert( QLatin1String( "people" ), QVariant::fromValue( container ) );
}

template<>
void insertPeopleVariants<QMap<QString, QVariant> >(Grantlee::Context &c)
{
  insertAssociatedPeopleVariants<QMap<QString, QVariant> >(c);
}

template<>
void insertPeopleVariants<QHash<QString, QVariant> >(Grantlee::Context &c)
{
  insertAssociatedPeopleVariants<QHash<QString, QVariant> >(c);
}

template<typename Container>
void testSequentialIteration(Grantlee::Context c)
{
  Grantlee::Engine engine;

  engine.setPluginPaths( QStringList() << QLatin1String( GRANTLEE_PLUGIN_PATH ) );

  {
    Grantlee::Template t1 = engine.newTemplate(
        QLatin1String( "{% for person in people %}{{ person.name }},{% endfor %}" ),
        QLatin1String( "people_template" ) );
    QCOMPARE(
      t1->render( &c ),
      QLatin1String( "Claire,Grant,Alan," ));
  }
}

template<typename Container>
void testSequentialIndexing(Grantlee::Context c)
{
  Grantlee::Engine engine;

  engine.setPluginPaths( QStringList() << QLatin1String( GRANTLEE_PLUGIN_PATH ) );

  {
    Grantlee::Template t1 = engine.newTemplate(
        QLatin1String( "{{ people.0.name }},{{ people.1.name }},{{ people.2.name }}," ),
        QLatin1String( "people_template" ) );
    QCOMPARE(
      t1->render( &c ),
      QLatin1String( "Claire,Grant,Alan," ));
  }
}

template<>
void testSequentialIndexing<QLinkedList<QVariant> >(Grantlee::Context)
{
  // No op
}

template<typename Container>
void doTestSequentialContainer_Variant()
{
  Grantlee::Context c;

  insertPeopleVariants<Container>(c);

  testSequentialIteration<Container>(c);
  testSequentialIndexing<Container>(c);
}

template<typename Container>
void testAssociativeValues(Grantlee::Context c, bool unordered = false)
{
  Grantlee::Engine engine;

  engine.setPluginPaths( QStringList() << QLatin1String( GRANTLEE_PLUGIN_PATH ) );

  {
    Grantlee::Template t1 = engine.newTemplate(
        QLatin1String( "{% for person in people.values %}({{ person.name }}:{{ person.age }}),{% endfor %}" ),
        QLatin1String( "people_template" ) );

    QString result = t1->render( &c );
    if (!unordered)
      QCOMPARE( result, QLatin1String( "(Claire:23),(Grant:32),(Alan:50)," ) );
    else {
      QVERIFY(result.size() == 33);
      QVERIFY(result.contains(QLatin1String( "(Claire:23),")));
      QVERIFY(result.contains(QLatin1String( "(Grant:32),")));
      QVERIFY(result.contains(QLatin1String( "(Alan:50),")));
    }
  }
}

template<typename Container>
void testAssociativeItems(Grantlee::Context c, bool unordered )
{
  Grantlee::Engine engine;

  engine.setPluginPaths( QStringList() << QLatin1String( GRANTLEE_PLUGIN_PATH ) );

  {
    Grantlee::Template t1 = engine.newTemplate(
        QLatin1String( "{% for item in people.items %}({{ item.1.name }}:{{ item.1.age }}),{% endfor %}" ),
        QLatin1String( "people_template" ) );
    QString result = t1->render( &c );
    if (!unordered)
      QCOMPARE( result, QLatin1String( "(Claire:23),(Grant:32),(Alan:50)," ) );
    else {
      QVERIFY(result.size() == 33);
      QVERIFY(result.contains(QLatin1String( "(Claire:23),")));
      QVERIFY(result.contains(QLatin1String( "(Grant:32),")));
      QVERIFY(result.contains(QLatin1String( "(Alan:50),")));
    }
  }
}

template<typename Container>
void doTestAssociativeContainer_Variant( bool unordered = false)
{
  Grantlee::Engine engine;

  engine.setPluginPaths( QStringList() << QLatin1String( GRANTLEE_PLUGIN_PATH ) );

  Grantlee::Context c;

  insertPeopleVariants<Container>(c);
  testAssociativeValues<Container>(c, unordered);
  testAssociativeItems<Container>(c, unordered);
}

void TestGenericTypes::testSequentialContainer_Variant()
{
  doTestSequentialContainer_Variant<QVariantList>();
  doTestSequentialContainer_Variant<QVector<QVariant> >();
  doTestSequentialContainer_Variant<QStack<QVariant> >();
  doTestSequentialContainer_Variant<QQueue<QVariant> >();
  doTestSequentialContainer_Variant<QLinkedList<QVariant> >();
}

void TestGenericTypes::testAssociativeContainer_Variant()
{
  doTestAssociativeContainer_Variant<QVariantMap>();
  doTestAssociativeContainer_Variant<QVariantHash>(true);
}

QTEST_MAIN( TestGenericTypes )
#include "testgenerictypes.moc"

