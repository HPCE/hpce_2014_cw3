Things to note about AWS
------------------------

First AWS is a commercial service, and they are in the
business of charging people money for access. This
course relies either on free instances, or on the $100
that I give you, but be aware that
there are consequences of leaving instances turned on
for long periods of time. Each type of instance
[costs money](http://aws.amazon.com/ec2/pricing/) per hour.
For example, if you accidentally leave a GPU instance running,
you will be charged $0.65 an hour until you stop it.

_AWS also offers ["spot pricing"](http://docs.aws.amazon.com/AWSEC2/latest/UserGuide/using-spot-instances-history.html)
where the price fluctuates according to demand - this is great for batch
compute, but I won't recommend it for this course as if the price
rises your machine may be terminated and you can lose work
unless you have good processes in place. If you look at the
pricing history it is quite interesting to try to work
out what people are doing. On the GPU instances people
occasionally spike the price to the on-demand rate, while
for c4.8xlarge the spot price quite sometimes spikes and
holds at 4x the on-demand rate._

Each instance can be in one of [a few states](http://docs.aws.amazon.com/AWSEC2/latest/UserGuide/ec2-instance-lifecycle.html):

1. Pending: once launched, the instance goes through some initial
   checks and is allocated to a machine.
   
2. Running: the machine is up and active, and you can SSH in and
   use it.
   
3. Stopped: the machine is not currently active, but the state of
   the hard disk is maintained, and can be run again.
   
4. Terminated: the machine is destroyed, including the hard disk.

Broadly speaking, you can think of the "running" state as
"using electricity", and the "stopped" state as "using up
a hard disk". AWS will always charge you for electricity
and disk space, but generally charges more for electricity.

You should all be eligible for the [one year special tier](http://aws.amazon.com/free/),
which gives you certain advantages:

- 750 hours of linux t2.micro server time. So you can leave
  a micro instance on all the time if you wanted to.
  
- 30GB of EBS storage. This is where your instance disks
  are stored, so you can have a few in the stopped state
  and not be charged.

The t2.micro instances are great for checking compilation
and automation, as you don't need to worry about cost.

Any other instances will eat into your $100, so you need
to be a bit careful about managing them. Things to
note are:

- If you have an instance running for t hours, you will
  be billed for ceil(t) hours. So a GPU instance used for
  10 minutes will cost the same as an hour.
  
- Stopping then starting an instance will cause the time
  to be rounded up to the nearest hour, then start a new
  billing period. So running an instance for 10 minutes,
  then stopping it, starting it, and running it for another
  10 minutes will cost the same as two hours.
  
- [Rebooting an instance](http://docs.aws.amazon.com/AWSEC2/latest/UserGuide/ec2-instance-reboot.html)
  does not result in a different instance, and so does not
  cause a new billing hour to start.

I have no more money to give you if yours runs out, and
you need to keep money for the later courseworks, so you
need a certain amount of planning in how you spend your money.
Don't start working with a GPU or large instance unless
you know you'll be able to spend a reasonable amount of
time with it, and use cheaper instances, lab/personal
machines and VMs to test build automation, compilation, and
testing whenever possible. Though if you consider $100 at $0.65
an hour, you're looking at 150+ hours, so there is plenty there.

Best practises for using AWS
----------------------------

1. *Always* check your instances are finished when you stop
   working with non-free instances. Check it has transitioned
   to the stopped or terminated state.
   
2. Protect your instance key-pairs.

3. Use the cheapest machine you can for the current purpose;
   testing OpenCL code for correctness doesn't necessarily
   need a GPU; checking that builds work can often be done
   in a tiny instance.
   
4. Plan your work; get everything possible done on a
   local machine (VM or native, linux or windows) first.

Getting an AWS Account
-------------------------

You need to have an Amazon account first (the kind
you use to buy books), then go to the [AWS site](http://aws.amazon.com/)
and create an AWS account.

It will ask you for a credit or debit card, but this
will not be charged if you only use free instances,
and/or stay within the $100 credit you'll get. As
I mentioned above, this is real money, but as long
as you manage your instances it won't cost you
anything.

Create a (tiny, free) Ubuntu 14.04 machine instance
---------------------------------------------------

### Step 1: Choose an Amazon Machine Image (AMI)

AWS: choose Ubuntu Server 14.04 LTS (HVM)

### Step 2: Choose an Instance Type

Select the free tier (you could choose a more expensive
one, but then you need to spend money).

Go to "Next: Configure Instance Details"

### Step 3: Configure Instance Details

You should be able to leave them at the defaults
(though it is interesting to look at all the options
by hovering over the (i) buttons).

### Step 4: Add Storage

You can leave at the defaults, but again, it is interesting
to read. If you ever need to work with big-ish data
then these options matter a lot.

### Step 5: Tag Instance

We don't need this, but it is useful if you have 20 instances
and you need to be able to identify which is which (Err,
don't create 20 instances unless you are rich).

### Step 6: Configure Security Group

This one is quite important. Your server will be alive on
the internet, open to the world, so you need to limit
access to you. We will use use one port, allowing SSH,
though we will allow it to be accessible from anywhere.

1. Select "Create a new security group". (It should be
    auto-selected, and the defaults listed below should
	be correct as well).

2. Make sure the "Type" on the left is SSH (Secure Shell Server).

3. Protocol and Port Range will then be fixed to TCP and 22.

4. For Source, specify Anywhere. This is so you can login from wherever
    your happen to be (so could anyone else, but SSH should stop them).
	
5. For security group name, choose something meaningful like
    "ssh-only".

Do Next: a dialogue should pop up saying
    "Select an existing or new key pair."

### 7. Selecting a key pair

First, do _not_ proceed without a key pair. These things are important,
as they are the thing that allows you to SSH into your instance.

1. Read the description of the key pair that it gives you.

2. Select "Create a new key pair". 

3. Choose a key pair name. I would suggest putting your name or
    login in it, for example, I use "m8pple-key-pair"

4. Download the key pair. This thing is important for as long
    as the instance is running, so keep it somewhere safe. 	
	However, you can always generate more key pairs if
	you lose one. If you are on a shared unix machine, change
	the permissions so that only you can access it:
	
	    chmod og-rwx m8pple-key-pair.pem
	
5. Finish the process, and your instance will launch.

Just re-emphasising the important of key pairs: they
are essentially the front-door key to your server.

Connecting to the instance
--------------------------

Use the "View Instances" button, or just go back to the
AWS dashboard (it doesn't matter how you get there).

You should now be able to see an instance running in
the dashboard, with a green symbol, and probably a
status that says "Initialising". If you click on that row,
then the bottom of the dashboard will show you details
about it.

The thing we need to connect is the DNS or IP address
of the instance - either will work to identify it. For example,
I have previously received:

    ec2-54-201-95-131.us-west-2.compute.amazonaws.com

as an instance, which is correspondingly at the IP address:

    54.201.95.131

To connect to the server, you need to SSH to it.

### Linux/Cygwin

You can ssh directly from the command line, using:

    ssh -i <path-to-your-key-pair> ubuntu@<dns-name-of-your-server>

That should drop you into a command line on the remote server.

### Windows (Putty)

There is a great ssh terminal for windows called
[PuTTY](http://www.chiark.greenend.org.uk/~sgtatham/putty/),
which I highly reccommend. To use it, you need to
convert the .pem file to a putty .ppk file:

1. Start PuTTYgen (one of the programs that comes with putty)

2. Conversions -> Import Key.

3. Select your .pem file and open it.

4. At this stage you can choose a passphrase using
   the "Key passphrase" box, which will be used to
   encrypt the private key. Personally I prefer to
   have a passphrase, as otherwise anyone who
   gets the key can get any of my running instances.
   However, you can leave it blank, and just protect
   the key well.

4. File -> Save Private Key.

5. You may get prompted about an empty passphrase,
   just ignore if you didn't want one.

6. Choose a .ppk file to save it as.

You can now start PuTTY itself. You might want to
set this up once and save it:

1. Session: "Host Name (or IP address)": Put the DNS name of your amazon instance.

2. Connection -> SSH -> Auth: Specify the private .ppk file you just created.

3. Connnection -> Data: In "Auto-login username" put "ubuntu".

4. Connection -> "Saved Sessions": Choose some name for this connection, e.g. "AWS", and
   hit save.
   
5. Hit "Open"

You should now be dropped into your remote server. If you switch
to a new instance you will need to change the host settings,
but the rest should stay the same.

Setting up the instance
-----------------------

By default, your instance has almost nothing on
it. Try running g++:

    g++ -v

And it will tell you it is not installed, but
does suggest how to install it:

	sudo apt-get g++

This involves two commands:

- [sudo](en.wikipedia.org/wiki/Sudo) A program for running commands
  as [root or adminstrator](http://en.wikipedia.org/wiki/Superuser).

- [apt-get](http://linux.die.net/man/8/apt-get) A package manager
  which handles the installation or removal of tools and libraries.
	
Similarly, try running git, make, and so on,
and you'll find you need to install them too.

TBB is also available as a package, but you
need to search for that:

	apt-cache search tbb

You should see four or five packages related
to tbb, but libtbb-dev should force everything
you need in:

    sudo apt-get libtbb-dev

Getting code over to your machine
---------------------------------

You now have a few options for getting code
over to your machine:

- Copying files over via scp (file transfer via SSH).
- `cat`ing files down the SSH connection (not really recommended,
  but occasionally very useful).
- Pulling code over via git.

I am going to recommend getting the code via git,
as it is a nice way of doing things, and makes it
easier to bring any patches you make in the test
environment back out to github. The main sticking point
is authentication, as your AWS instance will be able
to communicate with github, but doesn't have
access to your keys.

You could transfer your SSH keys over and use `ssh-agent`
remotely, but it is better to keep your keys where you
control them, using a method called [SSH agent forwarding](https://developer.github.com/guides/using-ssh-agent-forwarding/).

First, make sure you are currently authenticated
with github, by doing:

    ssh git@github.com

or the equivalent in PuTTY. If you receive something
like `Permission denied (publickey).`, then you haven't
got an agent set up. Start `ssh-agent` or `pageant`, load
your github SSH keys in (these are distinct from your
AWS keypair), then try again. Hopefully eventually you
will see something like:

    Hi m8pple! You've successfully authenticated, but GitHub does not provide shell access.

This shows that you successfully have agent
authentication working on your local machine. We
can now use authentication forwarding (`-A`) to allow
the remote server to access your local authentication
agent:

    ssh -A -i <path-to-your-key-pair> ubuntu@<dns-name-of-your-server>

You should end up on the remote server again, but if you
(within the SSH session, on the remote server) do:

	ssh git@github.com

You should see that you are authenticated with github
on the other machine.

You can now issue git clone command to get your
repository remotely, then do commit/push/pull as normal.

If you make modifications on the remote server,
don't forget to "push" any changes back into github,
and then (if necessary) to "pull" changes back down
to your normal working repository. For those who
are not used to the remote git, then the `git commit -a`
command is useful, as it auto-stages all your changes.

Editing code on the remote instance
-----------------------------------

I would not recommend doing much editing on
the remote instances, it should be more for
testing, tuning, and experimentation. But inevitably
you will need to change some source files, and
need some way of editing the files remotely (you
don't want to be pulling for each edit). There
is a command line editor called [nano](http://www.nano-editor.org/docs.php)
installed on pretty much all unix machines which
you can use to make small changes. For example,
to edit the file `wibble.cpp`, just do:

    nano wibble.cpp
    
You'll end up in an editor which behaves as you
would expect. Along the bottom are a number of
keyboard short-cuts, with `^` representing control.
The main ones you'll want are:

- `^X` (ctrl+x) : Quit the editor (it will prompt to save).
- `^O` (ctrl+o) : Write the file out without changing it.
- `^G` (ctrl+g) : Built-in documentation.

Other text editors can be used or installed (emacs, vim, ...),
but I would suggest nano for most tasks you will
encounter here.
