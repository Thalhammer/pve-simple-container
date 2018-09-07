# pve-simple-container
A small utility to allow docker like deployment of single application containers to a unmodified pve host.

What works:
* Building of minimal docker like containers based on JSON definition
* Upload to PVE and creation of LXC container using REST API

## How to use
You have to write a json definition describing your container image.
Once this is done all you have to do in order to build it is to run:
```pvesc container build```
This will unpack a base image which includes a init script and busybox, copy all your services files into the expected places, setup all configuration and repack it as a pve container template.
You can now create a pve container using the command:
```pvesc deploy```
This will upload the template to your pve host and create a container from it.