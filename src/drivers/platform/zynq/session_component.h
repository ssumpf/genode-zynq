/*
 * \brief  Platform driver - session component
 * \author Stefan Kalkowski
 * \date   2020-04-13
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVERS__PLATFORM__SESSION_COMPONENT_H_
#define _SRC__DRIVERS__PLATFORM__SESSION_COMPONENT_H_

#include <base/attached_rom_dataspace.h>
#include <base/env.h>
#include <base/heap.h>
#include <base/quota_guard.h>
#include <base/registry.h>
#include <base/session_object.h>
#include <os/dynamic_rom_session.h>
#include <os/session_policy.h>
#include <platform_session/platform_session.h>

#include <device_component.h>
#include <device_pd.h>
#include <device_owner.h>
#include <control_device.h>

namespace Driver {
	class Session_component;
	class Root;
}


class Driver::Session_component
:
	public  Session_object<Platform::Session>,
	public  Device_owner,
	private Registry<Driver::Session_component>::Element,
	private Dynamic_rom_session::Xml_producer
{
	public:

		using Session_registry = Registry<Session_component>;
		using Policy_version   = String<64>;

		Session_component(Env                          & env,
		                  Attached_rom_dataspace const & config,
		                  Device_model                 & devices,
		                  Session_registry             & registry,
		                  Control_devices              & control_devices,
		                  Label            const       & label,
		                  Resources        const       & resources,
		                  Diag             const       & diag,
		                  bool             const         info,
		                  Policy_version   const         version,
		                  bool             const         iommu);

		~Session_component();

		Heap      & heap();
		Device_pd & device_pd();

		bool matches(Device const &) const;

		Ram_quota_guard & ram_quota_guard() { return _ram_quota_guard(); }
		Cap_quota_guard & cap_quota_guard() { return _cap_quota_guard(); }

		void update_control_devices();
		void update_policy(bool info, Policy_version version);

		/**************************
		 ** Device Owner methods **
		 **************************/

		void enable_device(Device const &) override;
		void disable_device(Device const &) override;
		void update_devices_rom() override;

		/**************************
		 ** Platform Session API **
		 **************************/

		using Device_capability = Capability<Platform::Device_interface>;
		using Device_name       = Platform::Session::Device_name;

		Rom_session_capability devices_rom() override;
		Device_capability acquire_device(Device_name const &) override;
		Device_capability acquire_single_device() override;
		void release_device(Device_capability) override;
		Ram_dataspace_capability alloc_dma_buffer(size_t, Cache) override;
		void free_dma_buffer(Ram_dataspace_capability ram_cap) override;
		addr_t dma_addr(Ram_dataspace_capability) override;

	private:

		friend class Root;

		struct Dma_buffer : Registry<Dma_buffer>::Element
		{
			Ram_dataspace_capability const cap;
			addr_t dma_addr { 0 };
			size_t size;

			Dma_buffer(Registry<Dma_buffer> & registry,
			           Ram_dataspace_capability const cap,
			           size_t size)
			: Registry<Dma_buffer>::Element(registry, *this), cap(cap), size(size) {}
		};

		struct Control_device_domain : public  Control_device::Domain,
		                               private Registry<Control_device_domain>::Element
		{
			Registry<Dma_buffer> const & dma_buffers;

			Control_device_domain(Registry<Control_device_domain> & registry,
			                      Control_device                  & device,
			                      Allocator                       & md_alloc,
			                      Registry<Dma_buffer>      const & dma_buffers)
			: Control_device::Domain(device, md_alloc),
			  Registry<Control_device_domain>::Element(registry, *this),
			  dma_buffers(dma_buffers)
			{
				/* add DMA buffer ranges */
				dma_buffers.for_each([&] (Dma_buffer const & dma_buf) {
					add_range({ dma_buf.dma_addr, dma_buf.size }); });
			}

			~Control_device_domain()
			{
				/* remove DMA buffer ranges */
				dma_buffers.for_each([&] (Dma_buffer const & dma_buf) {
					remove_range({ dma_buf.dma_addr, dma_buf.size }); });
			}
		};

		using Domain_registry = Registry<Control_device_domain>;

		Env                          & _env;
		Attached_rom_dataspace const & _config;
		Device_model                 & _devices;

		Control_devices              & _control_devices;
		Device::Owner                  _owner_id    { *this };
		Constrained_ram_allocator      _env_ram     { _env.pd(),
		                                              _ram_quota_guard(),
		                                              _cap_quota_guard()  };
		Heap                           _md_alloc    { _env_ram, _env.rm() };
		Registry<Device_component>     _device_registry { };
		Registry<Dma_buffer>           _buffer_registry { };
		Domain_registry                _domain_registry { };
		Dynamic_rom_session            _rom_session { _env.ep(), _env.ram(),
		                                              _env.rm(), *this    };
		bool                           _info;
		Policy_version                 _version;
		bool const                     _iommu;
		Device_pd                      _device_pd { _env,
		                                            _md_alloc,
		                                            _ram_quota_guard(),
		                                            _cap_quota_guard(),
		                                            _iommu };

		Device_capability _acquire(Device & device);
		void              _release_device(Device_component & dc);
		void              _free_dma_buffer(Dma_buffer & buf);

		template <typename MATCH_FN, typename NONMATCH_FN>
		void _with_device_domain(Device::Name const & name,
		                        MATCH_FN          && match_fn,
		                        NONMATCH_FN       && nonmatch_fn)
		{
			bool exists = false;
			_domain_registry.for_each([&] (Control_device_domain & domain) {
				if (domain.device_name() == name) {
					match_fn(domain);
					exists = true;
				}
			});

			if (!exists)
				nonmatch_fn();
		}

		/*
		 * Noncopyable
		 */
		Session_component(Session_component const &);
		Session_component &operator = (Session_component const &);


		/*******************************************
		 ** Dynamic_rom_session::Xml_producer API **
		 *******************************************/

		void produce_xml(Xml_generator &xml) override;
};

#endif /* _SRC__DRIVERS__PLATFORM__SESSION_COMPONENT_H_ */