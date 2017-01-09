This is a small library of code, primarily for clique enumeration.

Compiling the Software
----------------------

On most systems, run

    make clean && make

If you prefer to use Visual Studio on Windows, you can generate
a suitable Visual Studio solution by

    installing premake5

and running

    premake5 vs2015

Extra build options under Linux
-------------------------------

By default, the library uses a dense bitvector for integer sets.
You may toggle the use of a sparse set representation by the following

    make clean && make sparse

Using and generating datasets
-----------------------------

A dataset is a set of graphs gathered into a common directory.
Each graph is a file in either text or binary DIMACS format.

A script is provided to apply an algorithm to a chosen dataset.
Here is it's general form:

    ./bench.sh algorithm dataset [output-file]

Here are examples of it's use:

    ./bench.sh bin/tomita dimacs
    ./bench.sh bin/tomita dimacs results.txt

Two datasets are provided:

    dimacs -- a collection of graphs from previous DIMACS challenges
    easy   -- a subset of the DIMACS graphs for quick testing

Two shell scripts are provided for generating synthetic graph datasets

    generate-dataset-TCS-article.sh
    -- generates a dataset of the same form as that used in Naudé (2016)

    generate-dataset-TCS-article-small.sh
    -- as above, but each category is restricted to 30 graph instances

Utilities provided
------------------

    degseq -- show degree sequence and graph properties
    gcut -- generate a graph with cliques separated by a cut vertex
    gk -- generate complete graphs
    gkbip -- generate complete bipartite graphs
    gnd -- generate graphs by Gnm model, with specific density
    gnm -- generate graphs by Gnm model
    gnp -- generate graphs by Gnp model
    
    tomita -- apply clique enumeration of Tomita et al. (2006)
    naude  -- apply clique enumeration of Naudé (2016)

