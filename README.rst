

==============================================================================
CALLBACK QUOTER taox11 progamming EXAMPLE             -- Kirthika Parameswaran
==============================================================================

This CORBA example is ported to **taox11** and **C++17** by Claus Klein.
==============================================================================

To generate the **makfiles** and **run-clang-tidy** type at the command prompt::

  brix11 gen build
  make Callback_Quoter_IDL
  make clean
  bear -- make all
  run-clang-tidy *_i.cpp *_Handler.cpp notifier.cpp consumer.cpp supplier.cpp


**bear:** Generates compilation database for **clang tooling**. see

https://github.com/rizsotto/Bear

Build requirements are:

You have to build **taox11** with **axcioma** on **linux**!

* https://github.com/ClausKlein/axcioma/blob/develop/build-taox11-on-linux.sh

Setup your enviroment with:

* https://github.com/ClausKlein/axcioma/blob/develop/.env_add.sh

-----------------------------------------------------------------------------

This is an distributed application which highlights the importance of the
**Callback** feature in helping meet the demands of various clients without them
having to poll continuously for input from the server.

There are three parts to the Callback Quoter Example.

1) Supplier
2) Notifier
3) Consumer


In detail:
==========

1) Supplier
-----------

--is the market feed daemon who keeps feeding the current stock information to
the **Notifier** periodically.  The timer handler has been used in the
implementation of the daemon process. It reads the current stock value from a
file and sends it to the **Notifier**.

2) Notifier
-----------

-- On getting information form the **Supplier**, it checks whether there are any
**Consumers** interested in the information and accordingly sends it to them.
The consumer object is registered with the notifier and the data is pushed to
the consumer using this reference.

3) Consumer
-----------

-- He is the stock broker interested in the stock values in the market.  He will
make decisions of selling only if the stock he is interested in has a price
greater than the threshold value he has set for that stock.  He just registers
himself with the **Notifier**. This saves the time he wastes in simply polling
for information from the **Notifier**.  This is the **Callback** feature in this
example.


Running the application:
========================

CASE I: Using the Naming Service
--------------------------------

a) Non-interactive

  Simply execute::

    ./run_test.pl


b) Interactive

.. tip:: Ofcourse after you have started the **Naming Service** in background!


There are 3 parts to it:
________________________

1) shell 1: type at the command prompt::

    ./notifier



2) shell 2: type at the command prompt::

    ./consumer

  register yourself with 'r'
  Enter the stockname and value.
  Now wait for information to arrive.

  You can unregister by typing 'u' and quit by typing 'q'.



3) shell 3: type at the command prompt::

    ./supplier -i example.stocks

  The -i option simply tells the daemon where to pick information from.

  The other option includes setting the period for the stock feed.

.. tip:: the contents of the input file per line should be: stockname and its price.
  See sample file: ./example.stocks


CASE II: Without using the Naming Service
-----------------------------------------

a) Non-interactive::

    ./run_test.pl -s -debug 2>&1 | grep -w Selling


b) Interactive

There are 3 parts to it:
________________________

1) shell 1: type at the command prompt::

    ./notifier -f notifier.ior -s -ORBDebugLevel 1



2) shell 2: type at the command prompt::

    ./consumer -f notifier.ior -s -a TAO -t 10 -ORBDebugLevel 1

  register yourself with 'r'
  Enter the stockname and value.
  Now wait for information to arrive.

  You can unregister by typing 'u' and quit by typing 'q'.



3) shell 3: type at the command prompt::

    ./supplier -i example.stocks -f notifier.ior -s -ORBDebugLevel 1

  The -i option simply tells the daemon where to pick information from.

  The other option includes setting the period for the stock feed.


-----------------------------------------------------------------------------

Happy troubleshooting!

