Name:       bluetooth-firmware-bcm
Summary:    firmware and tools for bluetooth
# Version/Release/Group are based on mobile package
Version:    0.2.30
Release:    0
Group:      Hardware Support/Handset
License:    Apache-2.0
Source0:    bluetooth-firmware-bcm-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires: model-build-features

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

%package msm8x26
Summary:    bcm firmware and tools for ms8x26
Group:      TO_BE/FILLED

%description msm8x26
bcm firmware and tools for Rinato 3g msm8x26

%prep
%setup -q -n bluetooth-firmware-bcm-%{version}

%build
export CFLAGS+=" -fpie -fvisibility=hidden"
export LDFLAGS+=" -Wl,--rpath=/usr/lib -Wl,--as-needed -Wl,--unresolved-symbols=ignore-in-shared-libs -pie"

%cmake \
%if "%{?tizen_profile_name}" == "wearable"
%if "%{?model_build_feature_model_name}" == "b2"
	-DTIZEN_WEARABLE_B2=YES \
	-DTIZEN_WEARABLE_B3=NO \
%elseif "%{?model_build_feature_model_name}" == "b3"
	-DTIZEN_WEARABLE_B2=NO \
	-DTIZEN_WEARABLE_B3=YES \
%endif
%elseif "%{?tizen_profile_name}" == "mobile"
	-DTIZEN_WEARABLE_B2=NO \
	-DTIZEN_WEARABLE_B3=NO \
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

%post c210
rm -rf %{_prefix}/etc/bluetooth/bt-dev-start.sh
ln -s %{_prefix}/etc/bluetooth/bt-dev-start-c210.sh %{_prefix}/etc/bluetooth/bt-dev-start.sh

%post e4412
rm -rf %{_prefix}/etc/bluetooth/bt-dev-start.sh
ln -s %{_prefix}/etc/bluetooth/bt-dev-start-e4412.sh %{_prefix}/etc/bluetooth/bt-dev-start.sh

%post msm8974
rm -rf %{_prefix}/etc/bluetooth/bt-dev-start.sh
ln -s %{_prefix}/etc/bluetooth/bt-dev-start-msm8974.sh %{_prefix}/etc/bluetooth/bt-dev-start.sh

%post msm8x26
rm -rf %{_prefix}/etc/bluetooth/bt-dev-start.sh
ln -s %{_prefix}/etc/bluetooth/bt-dev-start-msm8x26.sh %{_prefix}/etc/bluetooth/bt-dev-start.sh

%files c210
%manifest bluetooth-firmware-bcm.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool
%{_bindir}/setbd
%{_prefix}/etc/bluetooth/BCM4330B1_002.001.003.0221.0265.hcd
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-end.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-start-c210.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-set-addr.sh
/usr/share/license/%{name}

%files e4412
%manifest bluetooth-firmware-bcm.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool
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

%files msm8974
%manifest bluetooth-firmware-bcm.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool
%{_bindir}/setbd
%{_prefix}/etc/bluetooth/BCM4335B0_002.001.006.0233.0234_ORC_RedWood.hcd
%{_prefix}/etc/bluetooth/BCM4339_003.001.009.0030.0122_ORC_RedWood.hcd
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-end.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-start-msm8974.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-set-addr.sh
/usr/share/license/%{name}

%files msm8x26
%manifest bluetooth-firmware-bcm.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool
%{_bindir}/setbd
%{_prefix}/etc/bluetooth/BCM43342A1_001.002.003.1006.0000_Rintao_G3_ePA.hcd
%{_prefix}/etc/bluetooth/BCM4334W0_001.002.003.0014.0017_Ponte_Solo_Semco_B58_13.5dBm.hcd
%{_prefix}/etc/bluetooth/BCM4343A0_001.001.034.0048.0145_ORC_Ponte_Solo-3G.hcd
%{_prefix}/etc/bluetooth/BCM4343A1_001.002.009.0009.0012_ORC_Ponte_Solo-3G.hcd
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-end.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-start-msm8x26.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-set-addr.sh
/usr/share/license/%{name}
