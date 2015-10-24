Name:       bluetooth-firmware-bcm
Summary:    firmware and tools for bluetooth
# Version/Release/Group are based on mobile package
Version:    0.2.29
Release:    1
Group:      Hardware Support/Handset
License:    Apache-2.0
Source0:    bluetooth-firmware-bcm-%{version}.tar.gz
Source1:    bluetooth-hciattach@.service
Source2:    bluetooth-hci-device.service

BuildRequires:  cmake

%description
firmware and tools for bluetooth

%package c210
Summary:    c210 firmware and tools for bluetooth
Group:      TO_BE/FILLED

%description c210
c210 firmware and tools for bluetooth

%package e4412
Summary:    e4412 firmware and tools for bluetooth
Group:      TO_BE/FILLED

%description e4412
e4412 firmware and tools for bluetooth

%package msm8974
Summary:    msm8974 firmware and tools for bluetooth
Group:      TO_BE/FILLED

%description msm8974
firmware and tools for bluetooth for redwood msm8974

%package sprdtrum
Summary:    broadcom firmware and tools for Kiran Spreadtrum
Group:      TO_BE/FILLED

%description sprdtrum
broadcom bluetooth firmware and tools Kiran Spreadtrum

%prep
%setup -q -n bluetooth-firmware-bcm-%{version}

%build
export CFLAGS+=" -fpie -fvisibility=hidden"
export LDFLAGS+=" -Wl,--rpath=/usr/lib -Wl,--as-needed -Wl,--unresolved-symbols=ignore-in-shared-libs -pie"

%if "%{?tizen_profile_name}" == "wearable"
export CFLAGS="$CFLAGS -DTIZEN_WEARABLE"
%endif

%cmake \
%if "%{?tizen_profile_name}" == "wearable"
        -DTIZEN_WEARABLE=YES \
%else
%if "%{?tizen_profile_name}" == "mobile" || "%{?tizen_profile_name}" == "tv"
        -DTIZEN_WEARABLE=NO \
%endif
%endif

MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
cmake ./ -DCMAKE_INSTALL_PREFIX=%{_prefix} -DPLUGIN_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}

%make_install

mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}
cat %{_builddir}/%{name}-%{version}/LICENSE.Broadcom >> %{buildroot}/usr/share/license/%{name}

install -D -m 0644 %SOURCE1 %{buildroot}%{_libdir}/systemd/system/bluetooth-hciattach@.service
install -D -m 0644 %SOURCE2 %{buildroot}%{_libdir}/systemd/system/bluetooth-hci-device.service

%post c210
rm -rf %{_prefix}/etc/bluetooth/bt-dev-start.sh
ln -s %{_prefix}/etc/bluetooth/bt-dev-start-c210.sh %{_prefix}/etc/bluetooth/bt-dev-start.sh

%post e4412
rm -rf %{_prefix}/etc/bluetooth/bt-dev-start.sh
ln -s %{_prefix}/etc/bluetooth/bt-dev-start-e4412.sh %{_prefix}/etc/bluetooth/bt-dev-start.sh

%post msm8974
rm -rf %{_prefix}/etc/bluetooth/bt-dev-start.sh
ln -s %{_prefix}/etc/bluetooth/bt-dev-start-msm8974.sh %{_prefix}/etc/bluetooth/bt-dev-start.sh

%files c210
%manifest bluetooth-firmware-bcm.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool_4330b1
%{_bindir}/setbd
%{_prefix}/etc/bluetooth/BCM4330B1_002.001.003.0221.0265.hcd
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-end.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-start-c210.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-set-addr.sh
/usr/share/license/%{name}
%exclude %{_libdir}/systemd/system/bluetooth-hciattach@.service
%exclude %{_libdir}/systemd/system/bluetooth-hci-device.service

%files e4412
%manifest bluetooth-firmware-bcm.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool_4330b1
%exclude %{_bindir}/bcmtool
%{_bindir}/setbd
%if "%{?tizen_profile_name}" == "wearable"
%{_prefix}/etc/bluetooth/BCM20710A1_001.002.014.0059.0060.hcd
%{_prefix}/etc/bluetooth/BCM4334B0_002.001.013.1675.1676_B2_ORC.hcd
%{_prefix}/etc/bluetooth/BCM4334W_Rinato_TestOnly.hcd
%{_prefix}/etc/bluetooth/BCM4334W_001.002.003.0997.1027_B58_ePA.hcd
%exclude %{_prefix}/etc/bluetooth/BCM4334B0_002.001.013.0079.0081.hcd
%else
%{_prefix}/etc/bluetooth/BCM4334B0_002.001.013.0079.0081.hcd
%exclude %{_prefix}/etc/bluetooth/BCM20710A1_001.002.014.0059.0060.hcd
%exclude %{_prefix}/etc/bluetooth/BCM4334B0_002.001.013.1675.1676_B2_ORC.hcd
%exclude %{_prefix}/etc/bluetooth/BCM4334W_Rinato_TestOnly.hcd
%exclude %{_prefix}/etc/bluetooth/BCM4334W_001.002.003.0997.1027_B58_ePA.hcd
%endif
/usr/share/license/%{name}
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-end.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-start-e4412.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-set-addr.sh
%exclude %{_libdir}/systemd/system/bluetooth-hciattach@.service
%exclude %{_libdir}/systemd/system/bluetooth-hci-device.service

%files msm8974
%manifest bluetooth-firmware-bcm.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool_4330b1
%exclude %{_bindir}/bcmtool
%{_bindir}/setbd
%{_prefix}/etc/bluetooth/BCM4335B0_002.001.006.0233.0234_ORC_RedWood.hcd
%{_prefix}/etc/bluetooth/BCM4339_003.001.009.0030.0122_ORC_RedWood.hcd
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-end.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-start-msm8974.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-set-addr.sh
/usr/share/license/%{name}
%exclude %{_libdir}/systemd/system/bluetooth-hciattach@.service
%exclude %{_libdir}/systemd/system/bluetooth-hci-device.service

%post sprdtrum
rm -rf %{_prefix}/etc/bluetooth/bt-dev-start.sh
ln -s %{_prefix}/etc/bluetooth/bt-dev-start-sprdtrum.sh %{_prefix}/etc/bluetooth/bt-dev-start.sh

%files sprdtrum
%manifest bluetooth-firmware-bcm.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool
%exclude %{_bindir}/bcmtool_4330b1
%{_bindir}/setbd
%{_prefix}/etc/bluetooth/BCM4343A0_001.001.034.0058.0215_ORC_Kiran.hcd
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-end.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-start-sprdtrum.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-set-addr.sh
/usr/share/license/%{name}
%{_libdir}/systemd/system/bluetooth-hciattach@.service
%{_libdir}/systemd/system/bluetooth-hci-device.service
/etc/smack/accesses.d/bluetooth-firmware-bcm.rule
