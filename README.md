# rempack

RemPack is a friendly package manager for the ReMarkable 2 tablet.

This is targeted specifically at the ReMarkable 2, but it will likely compile fine for RM1 and other tablets targeted by rmkit. If you run this on another device, I'd love to hear about it!

The goal is to create a package manager that is as featureful as possible for the RM2.


### What is this?

This is a package manager heavily inspired by [Octopi](https://github.com/aarnt/octopi), the inarguable and undisputed best package manager available for Arch linux. It allows you to search, browse, and install packages from the [Toltec]() and [Entware]() repositories, which is the standard for ReMarkable tablets as well as similar devices.

Under the hood, the hard work is done by opkg, which is the native command-line package manager for these devices. This application mainly consists of a pleasant UI which invokes opkg commands behind the scenes. All of the advanced features rely on a custom parser that reads the opkg repo cache from disk and constructs an in-memory structure that can be searched and traversed rapidly. This is what enables searching and filtering with minimal delay.

This is my way of getting started with RM2 development. I wanted a better package manager so I'm making one.

This also will (eventually) be a guide to getting started with RM2 development, as well as a living demonstration of what it takes to build an application for these tablets.

### Why is this?

I want a package manager that makes discovering new applications pleasant and easy.

Due to the way opkg works, and the hardware limitations of the tablet, using opkg directly is *extremely* slow. This project aims to alleviate that as much as is possible.

The goal is to make the opkg repositories browsable, sortable, and searchable by anyone. If you like using the package manager, you'll probably install more programs, which makes package developers (me) happy.

### Goals

- [x] Stateful repo browsing
- [x] Aggregate repo browsing
- [x] Search engine
- [x] Filters
- [ ] What's new
- [ ] Repo manager
- [x] Seamless opkg repo update
- [ ] Update notifications
- [ ] Popularity contest
