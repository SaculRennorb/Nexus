#include "EAddonFlags.h"

EAddonFlags operator|(EAddonFlags lhs, EAddonFlags rhs)
{
	return static_cast<EAddonFlags>(
		static_cast<int>(lhs) |
		static_cast<int>(rhs)
		);
}

EAddonFlags operator&(EAddonFlags lhs, EAddonFlags rhs)
{
	return static_cast<EAddonFlags>(
		static_cast<int>(lhs) &
		static_cast<int>(rhs)
		);
}

EAddonFlags operator==(EAddonFlags lhs, EAddonFlags rhs)
{
	return lhs == rhs;
}

EAddonFlags operator!=(EAddonFlags lhs, EAddonFlags rhs)
{
	return lhs != rhs;
}