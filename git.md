Pushing to a remote git repository
==================================

Set up and pull
---------------

You now need to start working with your own
private remote repository in github. This
requires some extra steps, but most of the
time you can work locally: do your editing,
compilation and experiments within your
local repository, and only "push" to your
remote repository as necessary. Any local
commits will be stored up and remembered,
then sent to the remote copy at the next push.

You started the coursework by cloning the
remote origin repository:

``` text
https://github.com/HPCE/hpce_2014_cw3  (v1)
 |              
 |  Clone                ^
 V                     origin 
/home/dt10/hpce_2014-cw3 (v1)
```

Your local repository is then yours to work with,
and you can do a number of edits and commits. For
example, lets say you change and add some local
files, taking your local version up to "v4":

``` text
https://github.com/HPCE/hpce_2014_cw3  (v1)
 :
 :                        ^
 :                      origin 
/home/dt10/hpce_2014-cw3 (v4)
```

If I were to now make some changes to the origin
repository, the two would be out of sync. Let's
say I modified `readme.md`, taking my copy to
version "v7". You could then "pull" those changes,
which would get integrated into your local repository:

``` text
https://github.com/HPCE/hpce_2014_cw3  (v7)
 |
 |  git pull              ^
 V                      origin
/home/dt10/hpce_2014-cw3 (v9)
```

The version numbers I'm using are quite notional,
as git does not support linear dependencies, it
uses a DAG of changes.

Pushing to a remote private repository
--------------------------------------

You should have been given a remote repository
on github that you have read/write access to,
and is visible in the website. The name I used
was `hpce_2014_cw3_login`, so in my case it is
`hpce_2014_cw3_dt10`. I now want to add that
repository as a new place I can push to, while
keeping the link to the "origin" source (this
is actually what github will say as well if
you look at your repository).

I can use [`git remote add`](https://help.github.com/articles/adding-a-remote/)
to do this:

    git remote add private git@github.com:HPCE/hpce_2014_cw3_dt10.git

This has set up a new relationship between my local
repository and a different remote:

``` text
https://github.com/HPCE/hpce_2014_cw3  (v7)
 :
 :                        ^
 :                      origin
/home/dt10/hpce_2014-cw3 (v9)
 :                      private
 :                        v
 :
git@github.com:HPCE/hpce_2014_cw3_dt10.git (v0)
```

I can now "push" my local repository into the private
copy, using:

    git push -u private master

which does:

``` text
https://github.com/HPCE/hpce_2014_cw3  (v7)
 :
 :                        ^
 :                      origin
/home/dt10/hpce_2014-cw3 (v9)
 |                      private
 |  Push                  v
 V
git@github.com:HPCE/hpce_2014_cw3_dt10.git (v9)
```

You local and private repository are now in sync and
should be identical. You could now continue to make
edits and commits in your local repository, taking it
up to "v11" (again, these version numbers are artificial):

``` text
https://github.com/HPCE/hpce_2014_cw3  (v7)
 :
 :                         ^
 :                       origin
/home/dt10/hpce_2014-cw3 (v11)
 :                       private
 :                         v            
 :                                      
git@github.com:HPCE/hpce_2014_cw3_dt10.git (v9)
```

which could then be pushed back up to your private repo:

``` text
https://github.com/HPCE/hpce_2014_cw3  (v7)
 :
 :                         ^
 :                       origin
/home/dt10/hpce_2014-cw3 (v11)
 |                       private
 |                         v
 | git push private master
 V
git@github.com:HPCE/hpce_2014_cw3_dt10.git (v9)
```

If I were to make updates to the origin repository
taking it to v13, you could continue to integrate
those changes by pulling from "origin":

``` text
https://github.com/HPCE/hpce_2014_cw3  (v13)
 |
 |  git pull origin master
 |                          ^
 V                       origin
/home/dt10/hpce_2014-cw3 (v13)
 :                       private
 :                         v
 :
git@github.com:HPCE/hpce_2014_cw3_dt10.git (v9)
```

which would get integrated, then could be
pushed back to your private repository.

Working on a remote machine
---------------------------

One of the things you often need to do is to
work on a remote machine with different
hardware, for example AWS in our case. git
makes it easy to get a working copy which
can stay in sync.

So from your remote machine, do:

    ssh git@github.com

to check that you are correctly authenticated.

You can now clone your private github repository:

    git clone git@github.com:HPCE/hpce_2014_cw3_dt10.git

This will give you another "local" repository,
except it will be on another machine:

``` text
https://github.com/HPCE/hpce_2014_cw3  (v13)
 :
 :                         ^
 :                       origin
/home/dt10/hpce_2014-cw3 (v13)
 :                       private                   
 :                         v
 :
git@github.com:HPCE/hpce_2014_cw3_dt10.git (v13)
 |
 |  git clone ...          ^
 V                       origin
/home/dt10/hpce_2014-cw3 (v13)
```

The two "local" repositories are not directly
connected, but can communicate via the private
remote. So once you have tried your code out on
the remote server and made some commits there, taking
it up to v15, you can bring them back with a
push from the remote machine:

``` text
https://github.com/HPCE/hpce_2014_cw3  (v13)
 :
 :                         ^
 :                       origin
/home/dt10/hpce_2014-cw3 (v13)
 :                       private
 :                         v
 :
git@github.com:HPCE/hpce_2014_cw3_dt10.git (v15)
 ^
 |  git push origin        ^
 |                       origin
/home/dt10/hpce_2014-cw3 (v15)
```

From there, you can bring the changes back to
your local working repository using another pull.
