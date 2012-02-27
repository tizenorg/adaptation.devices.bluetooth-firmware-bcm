
Name:       bt-firmware-43xx
Summary:    Tools and scripts for Bluetooth stack
Version:    0.1.0
Release:    1
Group:      TO_BE/FILLED_IN
License:    GPL
Source0:    %{name}-%{version}.tar.gz

BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  cmake

%description
Tools and scripts for Bluetooth stack

%prep
%setup -q

%build
export CFLAGS+=" $CFLAGS -fpie"
export LDFLAGS+=" -Wl,--rpath=/usr/lib -Wl,--as-needed -Wl,--unresolved-symbols=ignore-in-shared-libs -pie"
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DPLUGIN_INSTALL_PREFIX=/usr


%install
rm -rf %{buildroot}
%make_install


%files
/etc/rc.d/init.d/*
/lib/firmware/*
/usr/bin/*
/usr/etc/bluetooth/*
